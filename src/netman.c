#include "../include/netman.h"
#include <string.h>


// Should perform any and all necessary initialization of netman
// netman must be ready to use after this.
void netman_init(netman_t * netman)
{
	/* TODO: check initialization to be done
	netman->current_rx_ack = 0;
	netman->current_rx_data = 0;
	netman->current_rx_fid = 0;
	//G netman->current_tx_ack = 0;
	//G netman->current_tx_data = 0;
	netman->current_tx_fid = 0;
	// assuming ground station communicates first
	netman->rx_state = 0;
	*/
	netman->tx_state = NOT_WAITING_FOR_ACK;
}

// Build an of2g DATA frame based on the given buffer of raw data, and
// store it in netman->current_tx_data
void netman_new_tx_bytes(netman_t * netman, unsigned char * buffer, size_t length)
{
	char fid = netman->current_tx_fid;
	of2g_build_data_frame(buffer, length, fid, netman->current_tx_data);
	netman->tx_state = WAITING_FOR_ACK;

}

// Build an of2g ACK frame based on the given data frame
void build_ack_frame (netman_t * netman, of2g_frame_t data_frame)
{
	// TODO: check if ack frame contains data (between length and checksum
	of2g_build_ack_frame(data_frame, netman->current_tx_ack);
	netman->current_tx_fid = of2g_get_ackid(data_frame); // TODO: Check logic again
}

// This function should handle the received `frame`, changing the state
// of `netman` and updating `current_*x_*` members appropriately
void netman_rx_frame(netman_t * netman, of2g_frame_t frame)
{
	// TODO: verify conditions for each state

	// if bad check sum
	if(!of2g_valid_frame(frame))
		netman->rx_state = BAD_CSUM;

	netman->rx_state = BAD_FID;	// TODO: Not sure how to know if bad FID
	// discard frame

	// get frametype
	of2g_frametype_t frametype = of2g_get_frametype(frame);
	if (frametype == OF2G_ACK) {

		// ACK of the received frame matches my FID, that means we received ack
		if (netman->current_tx_fid == of2g_get_ackid(frame))
		{
			netman->tx_state = NOT_WAITING_FOR_ACK;
		}
		// ackid matched last sent FID
		if(netman->current_tx_fid == of2g_get_ackid(frame))
		{
			netman->rx_state = NEW_ACK;
         memcpy(&netman->current_rx_ack, frame, sizeof(of2g_frame_t));
			// update FID
			++netman->current_rx_fid;
		}

		// TODO: check case where we receive the same ACK
	}
	else if (frametype == OF2G_DATA) { // TODO: Fix logic here
		if(netman->tx_state == NOT_WAITING_FOR_ACK) { // not waiting for ack, ignore fid for now
			netman->rx_state = NEW_DATA;
			netman->current_rx_fid = of2g_get_fid(frame);
         memcpy(&netman->current_rx_data, frame, sizeof(of2g_frame_t));
		}
		else {
			netman->rx_state = DUP_DATA;
			// ACK not received, send again
		}
	}

	else {
		// wasn't data or ack frame
	}




}
