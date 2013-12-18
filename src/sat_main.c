#include <stdio.h>
#include <assert.h>
#include <time.h>

#include <timer.h>
#include <Net2Com.h>

#include "../include/netman.h"
#include "../include/transceiver.h"

void loop_until_session_closed(netman_t *, Net2Com * net2com);
void loop_until_session_established(netman_t *, Net2Com * net2com);

int main()
{

   netman_t netman;
   netman_init(&netman);

   Net2Com net2com(Dnet_w_com_r, Dcom_w_net_r, Inet_w_com_r, Icom_w_net_r);

   while(1){

      //loop_until_session_established(&netman, &net2com);

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

         netman_rx_frame(netman,frame); // this will set the netman state and process the received frame
         transceiver_write(netman->current_tx_ack);
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
   unsigned char end_command = 0xFF;
   unsigned char infopipe_bytes = 0x01;
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
            printf("About to re-TX data over transceiver!!\n");
            transceiver_write(netman->current_tx_data);
            printf("Done re-TX data over transceiver!!\n");
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
         }
      }else{
         // Otherwise, logic tells us we most not be waiting for
         // an ACK. That means if we have new data to send, we can
         // send it!
         assert(netman->tx_state == NOT_WAITING_FOR_ACK);
         // If we have any new data to send...
         if(0 < (n_bytes = net2com->ReadFromDataPipe(buffer, BUFFLEN))){
            printf("Read from commander data pipe\n");
            // ... then we send it!
            netman_new_tx_bytes(netman, (unsigned char *)buffer, n_bytes);
            printf("About to TX data over transceiver!!\n");
            transceiver_write(netman->current_tx_data);
            printf("Done TX data over transceiver!!\n");
            // We also restart our timer so that we know how long
            // we've been waiting.
            printf("Starting resend timer at line %d\n", __LINE__);
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
         }
         else if(0 < (n_bytes = net2com->ReadFromInfoPipe(buffer, BUFFLEN))){
            // Read from info pipe
            printf("Read from commander infopipe\n");
            netman_new_tx_bytes(netman, (unsigned char *)buffer, n_bytes);
            printf("About to TX data over transceiver!!\n");
            transceiver_write(netman->current_tx_data);
            printf("Done TX data over transceiver!!\n");

            printf("Starting resend timer at line %d\n", __LINE__);
            timer_start(&resend_timer, RESEND_TIMEOUT, 0);
            
         }
      }

      // If there is new data incoming...
      if(transceiver_read(frame)){
         printf("New data (%s:%d)\n", __FILE__, __LINE__);
         // ... we process it.
         netman_rx_frame(netman, frame);

         // Then, depending on what we received, we take some
         // action.
         switch(netman->rx_state){
            case NEW_ACK:
               printf("  NEW_ACK with ackid = %02X, restarting window (%s:%d)\n", of2g_get_ackid(frame), __FILE__, __LINE__);
               // We don't need to do anything other than keep
               // the window open.
               timer_start(&window_timer, WINDOW_TIMEOUT, 0);
               break;
            case NEW_DATA:
               printf("  NEW_DATA with fid = %02X, sending ACK, writing commander pipe (%s:%d)\n", of2g_get_fid(frame), __FILE__, __LINE__);
               // We need to acknowledge that we received the data
               // ok, send the data to the commander, and finally
               // we need to make sure the window stays open.
               transceiver_write(netman->current_tx_ack);
               n_bytes = of2g_get_data_content(netman->current_rx_data, (unsigned char *)buffer);
		printf("n_bytes = '%zu'\n" , n_bytes);
//		printf("Buffer = '%s' ", buffer);	

	       net2com->WriteToInfoPipe(infopipe_bytes);
	       printf("\nWrote to info pipe: 0x%02X\n", infopipe_bytes);
               net2com->WriteToDataPipe(buffer, n_bytes);
    	       printf("Received Comand: ");
		for(uint8_t i = 0; i < n_bytes; ++i){
		   uint8_t c = buffer[i];
		   if(c >= ' ' && c <= '~'){
			putchar(c);
			} else {
				printf(" 0x%02X ", c);
			}
		}

	       net2com->WriteToInfoPipe(end_command); 
	       printf("\nWrote to info pipe: 0x%02X\n", end_command);
               timer_start(&window_timer, WINDOW_TIMEOUT, 0);
               break;
            case DUP_DATA:
               printf("  DUP_DATA, resending ACK (%s:%d)\n", __FILE__, __LINE__);
               // Duplicate DATA? They must not have got our ACK, lets
               // send it again.
               transceiver_write(netman->current_tx_ack);
               // TODO - does this keep the window open??
               break;
            case BAD_FID:
            case BAD_CSUM:
            default:
               printf("  SOME KIND OF GARBAGE, ignore it (%s:%d)\n", __FILE__, __LINE__);
               // Some kind of garbage, who cares, just ignore it.
               break;
         }
      }
   }
}


