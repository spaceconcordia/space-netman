#include <stdio.h>
#include <assert.h>

#include <Net2Com.h>

#include "../include/netman.h"
#include "../include/timer.h"
#include "../include/transceiver.h"

void loop_until_session_closed(netman_t *, Net2Com * net2com);
void loop_until_session_established(netman_t *, Net2Com * net2com);

int main()
{

   netman_t netman;
   netman_init(&netman);

   Net2Com net2com(PIPE_TWO, PIPE_ONE, PIPE_FOUR, PIPE_THREE);

   while(1){

      loop_until_session_established(&netman, &net2com);

      loop_until_session_closed(&netman, &net2com);
   }

   return 0;
}

void loop_until_session_established(netman_t * netman, Net2Com * net2com){

   const size_t BUFFLEN = 256;
   char buffer[BUFFLEN]; // TODO - exact max size
   size_t n_bytes;

   of2g_frame_t frame;

   while(1){

      if(
            transceiver_read(frame) &&
            of2g_valid_frame(frame) &&
            of2g_get_frametype(frame) == OF2G_DATA
      ){

         // transceiver_write(netman->current_tx_ack); // TODO - handle intial ACK and session starting!!!
         n_bytes = of2g_get_data_content(netman->current_rx_data, (unsigned char *)buffer);
         net2com->WriteToDataPipe(buffer, n_bytes);

         break;
      }
   }
}

void loop_until_session_closed(netman_t * netman, Net2Com * net2com){

   const size_t BUFFLEN = 256;
   char buffer[BUFFLEN]; // TODO - exact max size
   size_t n_bytes;

   of2g_frame_t frame;

   timer_t window_timer = timer_get();
   timer_t resend_timer = timer_get();

   const uint32_t RESEND_TIMEOUT =      10 * 1000;
   const uint32_t WINDOW_TIMEOUT = 15 * 60 * 1000;

   // We continue in the loop as long as the window is open
   while( ! timer_complete(&window_timer) ){

      // If we're waiting for an ACK...
      if(netman->tx_state == WAITING_FOR_ACK){
         // ...and we've been waiting for a while...
         if( timer_complete(&resend_timer) ){
            // ...then we're impatient and we resend our data.
            transceiver_write(netman->current_tx_data);
         }
      }else{
         // Otherwise, logic tells us we most not be waiting for
         // an ACK. That means if we have new data to send, we can
         // send it!
         assert(netman->tx_state == NOT_WAITING_FOR_ACK);
         // If we have any new data to send...
         if(0 < (n_bytes = net2com->ReadFromDataPipe(buffer, BUFFLEN))){
            // ... then we send it!
            netman_new_tx_bytes(netman, (unsigned char *)buffer, n_bytes);
            transceiver_write(netman->current_tx_data); 
            // We also restart our timer so that we know how long
            // we've been waiting.
            timer_start(&resend_timer, RESEND_TIMEOUT);
         }
      }

      // If there is new data incoming...
      if(transceiver_read(frame)){
         // ... we process it.
         netman_rx_frame(netman, frame);

         // Then, depending on what we received, we take some
         // action.
         switch(netman->rx_state){
            case NEW_ACK:
               // We don't need to do anything other than keep
               // the window open.
               timer_start(&window_timer, RESEND_TIMEOUT);
               break;
            case NEW_DATA:
               // We need to acknowledge that we received the data
               // ok, send the data to the commander, and finally
               // we need to make sure the window stays open.
               transceiver_write(netman->current_tx_ack);
               n_bytes = of2g_get_data_content(netman->current_rx_data, (unsigned char *)buffer);
               net2com->WriteToDataPipe(buffer, n_bytes);
               timer_start(&window_timer, WINDOW_TIMEOUT);
               break;
            case DUP_DATA:
               // Duplicate DATA? They must not have got our ACK, lets
               // send it again.
               transceiver_write(netman->current_tx_ack); 
               // TODO - does this keep the window open??
               break;
            case BAD_FID:
            case BAD_CSUM:
            default:
               // Some kind of garbage, who cares, just ignore it.
               break;
         }
      }
   }
}
