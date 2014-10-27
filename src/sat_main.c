#include <stdio.h>
#include <assert.h>
#include <time.h>

#include <timer.h>
#include <Net2Com.h>
#include <shakespeare.h>
#include "../include/netman.h"
#include "../include/transceiver.h"

using namespace std;
const char * LOGNAME = cs1_systems[CS1_NETMAN];
#define LOG_BUFFER_SIZE 100

void loop_until_session_closed(netman_t *, Net2Com * net2com);
void loop_until_session_established(netman_t *, Net2Com * net2com);

char log_buffer[LOG_BUFFER_SIZE]={0};

int main()
{
   Shakespeare::log(Shakespeare::NOTICE, "Netman", "Starting");

   netman_t netman;
   netman_init(&netman);

   Net2Com net2com(Dnet_w_com_r, Dcom_w_net_r, Inet_w_com_r, Icom_w_net_r);

   while(1){

      loop_until_session_established(&netman, &net2com);
      Shakespeare::log(Shakespeare::NOTICE, "Netman", "Session established");

      loop_until_session_closed(&netman, &net2com);
      Shakespeare::log(Shakespeare::NOTICE, "Netman", "Session closed");
   }

   return 0;
}

void loop_until_session_established(netman_t * netman, Net2Com * net2com)
{
   const size_t BUFFLEN = 256;
   char buffer[BUFFLEN]; // TODO - exact max size
   size_t n_bytes;
   unsigned char end_command = 0xFF;

   of2g_frame_t frame;
   Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Waiting for session to establish...");

   while (1)
   {
      if(
            transceiver_read(frame) &&
            of2g_valid_frame(frame) &&
            of2g_get_frametype(frame) == OF2G_DATA
      )
      {
         netman_rx_frame(netman,frame); // this will set the netman state and process the received frame
         transceiver_write(netman->current_tx_ack);
         n_bytes = of2g_get_data_content(netman->current_rx_data, (unsigned char *)buffer);

         memset(log_buffer,0,LOG_BUFFER_SIZE);
         snprintf(log_buffer,LOG_BUFFER_SIZE,"Received %zu bytes from ground station",n_bytes);
         Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);

         net2com->WriteToInfoPipe(n_bytes);
         memset(log_buffer,0,LOG_BUFFER_SIZE);
         snprintf(log_buffer,LOG_BUFFER_SIZE,"Wrote to info pipe: %zu",n_bytes);
         Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
         net2com->WriteToDataPipe(buffer, n_bytes);

         Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Netman received Command: ");
         for(uint8_t i = 0; i < n_bytes; ++i){
            uint8_t c = buffer[i];
            memset(log_buffer,0,LOG_BUFFER_SIZE);
            if(c >= ' ' && c <= '~') {
                putchar(c);
            } else {
                snprintf(log_buffer,LOG_BUFFER_SIZE," 0x%02X ", c);
                Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
            }
         }

         net2com->WriteToInfoPipe(end_command);
         memset(log_buffer,0,LOG_BUFFER_SIZE);
         snprintf(log_buffer,LOG_BUFFER_SIZE,"Wrote to info pipe: 0x%02X", end_command);
         break;
      }
   }
}

void loop_until_session_closed(netman_t * netman, Net2Com * net2com)
{
   const size_t BUFFLEN = 256;
   char buffer[BUFFLEN]; // TODO - exact max size
   size_t n_bytes;
   unsigned char end_command = 0xFF;
   of2g_frame_t frame;

   timer_t window_timer = timer_get();
   timer_t resend_timer = timer_get();

   // values in seconds
   const uint32_t RESEND_TIMEOUT =      10;
   const uint32_t WINDOW_TIMEOUT = 15 * 60;
   Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Session started!");
   timer_start(&window_timer, WINDOW_TIMEOUT, 0);

   // We continue in the loop as long as the window is open
   while ( ! timer_complete(&window_timer) )
   {
      // If we're waiting for an ACK...
      if ( netman->tx_state == WAITING_FOR_ACK )
      {
         // ...and we've been waiting for a while...
         if ( timer_complete(&resend_timer) )
         {
            // ...then we're impatient and we resend our data.
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"About to re-TX data over transceiver!!");
            transceiver_write(netman->current_tx_data);
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Done re-TX data over transceiver!!");
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
         }
      } 
      else
      {
         // Otherwise, logic tells us we most not be waiting for
         // an ACK. That means if we have new data to send, we can
         // send it!
         assert(netman->tx_state == NOT_WAITING_FOR_ACK);
         // If we have any new data to send...
         if( 0 < ( n_bytes = net2com->ReadFromDataPipe(buffer, BUFFLEN) ) )
         {
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Read from commander data pipe");
            // ... then we send it!
            netman_new_tx_bytes(netman, (unsigned char *)buffer, n_bytes);
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"About to TX data over transceiver!!");
            transceiver_write(netman->current_tx_data);
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Done TX data over transceiver!!");
            // We also restart our timer so that we know how long
            // we've been waiting.
            memset(log_buffer,0,LOG_BUFFER_SIZE);
            snprintf(log_buffer,LOG_BUFFER_SIZE,"Starting resend timer at line %d", __LINE__);
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
         }
         else if ( 0 < (n_bytes = net2com->ReadFromInfoPipe(buffer, BUFFLEN) ) )
         {
            // Read from info pipe
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Read from commander infopipe");
            // log or output what went wrong depending on info pipe data
            if ( buffer[0] == 0x31 ) 
            {
              memset(log_buffer,0,LOG_BUFFER_SIZE);
              snprintf(log_buffer,LOG_BUFFER_SIZE,"Read %c from infopipe, ERROR creating command\n", buffer[0]);
              Shakespeare::log(Shakespeare::ERROR,LOGNAME,log_buffer);
              Shakespeare::log(Shakespeare::ERROR, LOGNAME, "Commander could not create command");
              sprintf(buffer,"Error creating command");
              netman_new_tx_bytes(netman, (unsigned char *)buffer, strlen(buffer));
            }
            else if ( buffer[0] == 0x32 ) {
              memset(log_buffer,0,LOG_BUFFER_SIZE);
              snprintf(log_buffer,LOG_BUFFER_SIZE,"Read %d from infopipe, ERROR executing command\n", buffer[0]);
              Shakespeare::log(Shakespeare::ERROR,LOGNAME,log_buffer);
              Shakespeare::log(Shakespeare::ERROR,LOGNAME, "Commander could not execute command");
              sprintf(buffer,"Error executing command");
              netman_new_tx_bytes(netman, (unsigned char *)buffer, strlen(buffer));
            }

            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"About to TX data over transceiver!!");
            transceiver_write(netman->current_tx_data);
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Done TX data over transceiver!!");

            memset(log_buffer,0,LOG_BUFFER_SIZE);
            snprintf(log_buffer,LOG_BUFFER_SIZE,"Starting resend timer at line %d\n", __LINE__);
            Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
            // TODO Read and log to tell if data completely sent
         }
      }

      // If there is new data incoming...
      if ( transceiver_read(frame) ) {
         memset(log_buffer,0,LOG_BUFFER_SIZE);   
         snprintf(log_buffer,LOG_BUFFER_SIZE,"New data (%s:%d)", __FILE__, __LINE__);
         Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
         // ... we process it.
         netman_rx_frame(netman, frame);

         // Then, depending on what we received, we take some
         // action.
         switch ( netman->rx_state ) 
         {
            case NEW_ACK:
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"  NEW_ACK with ackid = %02X, restarting window (%s:%d)", of2g_get_ackid(frame), __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
               // We don't need to do anything other than keep
               // the window open.
               timer_start(&window_timer, WINDOW_TIMEOUT, 0);
               break;
            case NEW_DATA:
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"  NEW_DATA with fid = %02X, sending ACK, writing commander pipe (%s:%d)", of2g_get_fid(frame), __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
               // We need to acknowledge that we received the data
               // ok, send the data to the commander, and finally
               // we need to make sure the window stays open.
               transceiver_write(netman->current_tx_ack);
               n_bytes = of2g_get_data_content(netman->current_rx_data, (unsigned char *)buffer);
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"Received %zu bytes from ground station" , n_bytes);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);

               net2com->WriteToInfoPipe(n_bytes);
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"Wrote to info pipe: %zu", n_bytes);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);

               net2com->WriteToDataPipe(buffer, n_bytes);
               
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Netman received Comand: ");
               for(uint8_t i = 0; i < n_bytes; ++i){
                   uint8_t c = buffer[i];
                   memset(log_buffer,0,LOG_BUFFER_SIZE);   
                   if ( c >= ' ' && c <= '~' ) 
                   {
                     putchar(c);
                   } 
                   else {
                     snprintf(log_buffer,LOG_BUFFER_SIZE," 0x%02X ", c);
                     Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
                   }
               }

               net2com->WriteToInfoPipe(end_command);
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"Wrote to info pipe: 0x%02X", end_command);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
               timer_start(&window_timer, WINDOW_TIMEOUT, 0);
               break;

            case DUP_DATA:
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"  DUP_DATA, resending ACK (%s:%d)\n", __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
               // Duplicate DATA? They must not have got our ACK, lets
               // send it again.
               transceiver_write(netman->current_tx_ack);
               // TODO - does this keep the window open??
               break;

            case BAD_FID:

            case BAD_CSUM:

            default:
               memset(log_buffer,0,LOG_BUFFER_SIZE);   
               snprintf(log_buffer,LOG_BUFFER_SIZE,"  SOME KIND OF GARBAGE, ignore it (%s:%d)\n", __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,LOGNAME,log_buffer);
               // Some kind of garbage, who cares, just ignore it.
               break;
         }
      }
   }
   Shakespeare::log(Shakespeare::NOTICE,LOGNAME,"Session ended!");
}
