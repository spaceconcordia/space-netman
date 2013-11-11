#include <stdio.h>
#include <assert.h>

void loop_until_session_closed();
void loop_until_session_established();

int main()
{

   netman_t netman;
   netman_init(&netman);

   while(1){

      loop_until_session_established(&netman);

      loop_until_session_closed(&netman);
   }

   return 0;
}

void loop_until_session_established(netman_t * netman){

   of2g_frame_t frame;

   while(1){

      if(
            he_read(&frame) &&
            of2g_valid_frame(&frame) &&
            of2g_get_frametype(&frame) == OF2G_DATA
      ){
         init_session();
         write_commander_pipe();
         break;
      }
   }
}

void loop_until_session_closed(netman_t * netman){

   unsigned char buffer[256]; // TODO - exact max size

   // create 2 timers
   timer_t window_timer = timer_get();
   timer_t resend_timer = timer_get();

   // We continue in the loop as long as the window is open
   while( ! timer_complete(&window_timer) ){

      // If we're waiting for an ACK...
      if(netman->tx_state == WAITING_FOR_ACK){
         // ...and we've been waiting for a while...
         if( timer_complete(&resend_timer) ){
            // ...then we're impatient and we resend our data.
            he_write(netman->current_tx_data);
         }
      }else{
         // Otherwise, logic tells us we most not be waiting for
         // an ACK. That means if we have new data to send, we can
         // send it!
         assert(netman->tx_state == NOT_WAITING_FOR_ACK);
         // If we have any new data to send...
         if( read_commander_output(buffer) ){         // TODO
            // ... then we send it!
            netman_new_tx_bytes(netman, buffer, length);
            he_write(netman->current_tx_data); 
            // We also restart our timer so that we know how long
            // we've been waiting.
            timer_start(&resend_timer, RESEND_TIMEOUT);
         }
      }

      // If there is new data incoming...
      if(he_read(&frame)){
         // ... we process it.
         netman_new_rx_frame(netman, &frame);

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
               he_write(netman->current_tx_ack); 
               write_commander_pipe(); // TODO
               timer_start(&window_timer, WINDOW_TIMEOUT);
               break;
            case DUP_DATA:
               // Duplicate DATA? They must not have got our ACK, lets
               // send it again.
               he_write(netman->current_tx_ack); 
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
