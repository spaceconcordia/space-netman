#include <stdio.h>

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

   of2g_frame_t frame

   while(1){
      poll_he(&frame);

      if(
            of2g_good_checksum(&frame) &&
            of2g_get_frametype(&frame) == OF2G_DATA
      ){
         init_session();
         write_commander_pipe();
         break;
      }
   }
}

void loop_until_session_closed(netman_t * netman){

   uint8_t buffer[256]; // TODO - exact max size

   while(window_open()){  

      if(netman->tx_state == WAITING_FOR_ACK){
         if( time_for_resend() ){                     // TODO
            send_of2g_frame(netman->current_tx_data);
         }
      }else{
         if( read_commander_output(buffer) ){         // TODO
            build_data_frame(netman, buffer, length); // TODO
            send_of2g_frame(netman->current_tx_data); 
         }
      }

      poll_he(); // TODO

      switch(netman->rx_state){
         case NEW_ACK:
            // FID has already been updated by netman
            break;
         case NEW_DATA:
            write_commander_pipe(); // TODO
            build_ack_frame(netman, netman->current_rx_data);
            send_of2g_frame(netman->current_tx_ack); 
            break;
         case DUP_DATA:
            send_of2g_frame(netman->current_tx_ack); 
            break;
         case BAD_FID:
         case BAD_CSUM:
         default: ;
      }
   }
}
