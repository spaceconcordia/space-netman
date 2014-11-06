#include <stdio.h>
#include <assert.h>
#include <time.h>

#include <timer.h>
#include <Net2Com.h>
#include <NamedPipe.h>

#include <shakespeare.h>
#include "../include/netman.h"
#include "../include/transceiver.h"

#define PROCESS "GROUND_NETMAN"
#define LOG_ENTRY_SIZE 100
#define MAX_QUEUE_SIZE 100

void loop_until_session_closed(netman_t *);

static NamedPipe gnd_input("/home/pipes/gnd-input");

of2g_frame_t command_queue[MAX_QUEUE_SIZE];

int main()
{
   Shakespeare::log(Shakespeare::NOTICE,PROCESS,"Starting ground netman");
   if(!gnd_input.Exist()) gnd_input.CreatePipe();

   netman_t netman;
   netman_init(&netman);

   while(1){
      Shakespeare::log(Shakespeare::NOTICE,PROCESS,"Starting loop_until_session_closed");
      loop_until_session_closed(&netman);
   }

   return 0;
}

void loop_until_session_closed(netman_t * netman)
{
   const size_t BUFFLEN = 256;
   char buffer[BUFFLEN]; // TODO - exact max size
   size_t n_bytes;
   char output[LOG_ENTRY_SIZE]={0}; // output buffer to be reused for shakespeare logging

   of2g_frame_t frame;

   timer_t window_timer = timer_get();
   timer_t resend_timer = timer_get();

   // values in seconds
   const uint32_t RESEND_TIMEOUT =      10;
   const uint32_t WINDOW_TIMEOUT = 15 * 60;

   timer_start(&window_timer, WINDOW_TIMEOUT, 0);

   // We continue in the loop as long as the window is open
   while( ! timer_complete(&window_timer) ){

      // If we're waiting for an ACK...
      if(netman->tx_state == WAITING_FOR_ACK){
         // ...and we've been waiting for a while...
         if( timer_complete(&resend_timer) ){
            // ...then we're impatient and we resend our data.
            Shakespeare::log(Shakespeare::NOTICE,PROCESS,"About to re-TX data over transceiver!!");
            transceiver_write(netman->current_tx_data);
            Shakespeare::log(Shakespeare::NOTICE,PROCESS,"Done re-TX data over transceiver!!");
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
         }
      }else{
         // Otherwise, logic tells us we most not be waiting for
         // an ACK. That means if we have new data to send, we can
         // send it!
         assert(netman->tx_state == NOT_WAITING_FOR_ACK);
         // If we have any new data to send...

         n_bytes = gnd_input.ReadFromPipe(buffer, BUFFLEN);

         if(0 < n_bytes){
            // ... then we send it!
            netman_new_tx_bytes(netman, (unsigned char *)buffer, n_bytes);
            Shakespeare::log(Shakespeare::NOTICE,PROCESS,"About to TX data over transceiver!!");
            transceiver_write(netman->current_tx_data);
            Shakespeare::log(Shakespeare::NOTICE,PROCESS,"Done TX data over transceiver!!");
            // We also restart our timer so that we know how long
            // we've been waiting.
            memset(output,0,LOG_ENTRY_SIZE);
            snprintf(output,40,"Starting resend timer at line %d", __LINE__);
            Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
         }
      }

      // If there is new data incoming...
      if(transceiver_read(frame)){
         memset(output,0,LOG_ENTRY_SIZE);
         snprintf(output,LOG_ENTRY_SIZE,"New data (%s:%d)", __FILE__, __LINE__);
         Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);
         // ... we process it.
         netman_rx_frame(netman, frame);

         // Then, depending on what we received, we take some
         // action.
         switch(netman->rx_state){
            case NEW_ACK:
               memset(output,0,LOG_ENTRY_SIZE);
               snprintf(output,LOG_ENTRY_SIZE,"  NEW_ACK with ackid = %02X, restarting window (%s:%d)", of2g_get_ackid(frame), __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);
               // We don't need to do anything other than keep
               // the window open.
               timer_start(&window_timer, WINDOW_TIMEOUT, 0);
               break;
            case NEW_DATA:
               memset(output,0,LOG_ENTRY_SIZE);
               snprintf(output,LOG_ENTRY_SIZE,"  NEW_DATA with fid = %02X, sending ACK (%s:%d)\n", of2g_get_fid(frame), __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);

               // We need to acknowledge that we received the data
               // ok, send the data to the commander, and finally
               // we need to make sure the window stays open.
#ifdef CS1_DEBUG
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,"    TX'ing ACK... ");
#endif
               transceiver_write(netman->current_tx_ack);
               // TODO log if this didn't work
#ifdef CS1_DEBUG
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,"    done\n");
#endif

               n_bytes = of2g_get_data_content(netman->current_rx_data, (unsigned char *)buffer);
#ifdef CS1_DEBUG
               memset(output,0,LOG_ENTRY_SIZE);
               snprintf(output,LOG_ENTRY_SIZE," Data received from satellite with num of bytes %zu: ", n_bytes);
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);

               memset(output,0,LOG_ENTRY_SIZE);
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,buffer);
               //TODO - print the buffer in hex
#endif
               timer_start(&window_timer, WINDOW_TIMEOUT, 0);
               break;
            case DUP_DATA:
#ifdef CS1_DEBUG
               memset(output,0,LOG_ENTRY_SIZE);
               snprintf(output,LOG_ENTRY_SIZE,"  DUP_DATA, resending ACK (%s:%d)\n", __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);
#endif
               // Duplicate DATA? They must not have got our ACK, lets
               // send it again.
               transceiver_write(netman->current_tx_ack);
               // TODO - does this keep the window open??
               break;
            case BAD_FID:
            case BAD_CSUM:
            default:
#ifdef CS1_DEBUG
               memset(output,0,LOG_ENTRY_SIZE);
               snprintf(output,LOG_ENTRY_SIZE,"  SOME KIND OF GARBAGE, ignore it (%s:%d)\n", __FILE__, __LINE__);
               Shakespeare::log(Shakespeare::NOTICE,PROCESS,output);
#endif
               assert(0);
               // Some kind of garbage, who cares, just ignore it.
               break;
         }
      }
   }
}

