#include "../include/netman.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char g_dataFID = 0x01;

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
	netman->current_tx_fid = g_dataFID;
	of2g_build_data_frame(buffer, length, g_dataFID, netman->current_tx_data);
	netman->tx_state = WAITING_FOR_ACK;
	if(g_dataFID == 255)
		g_dataFID = 0x00;

	g_dataFID += 1;
}

// Build an of2g ACK frame based on the given data frame
void build_ack_frame (netman_t * netman, of2g_frame_t data_frame)
{
	of2g_build_ack_frame(data_frame, netman->current_tx_ack);
	netman->current_tx_fid = of2g_get_fid(netman->current_tx_ack);
}

// This function should handle the received `frame`, changing the state
// of `netman` and updating `current_*x_*` members appropriately
void netman_rx_frame(netman_t * netman, of2g_frame_t frame)
{

	if(!of2g_valid_frame(frame))
		netman->rx_state = BAD_CSUM;

	// FID cannot be 0, otherwise the ackid of it would be 0
	if(of2g_get_fid(frame) == 0x0)
		netman->rx_state = BAD_FID;

	of2g_frametype_t frametype = of2g_get_frametype(frame);
	if (frametype == OF2G_ACK)
	{
		// Received the same ack, still waiting for new
		// ACK of the received frame matches my last sent FID, that means we received ack
		if (netman->current_tx_fid == of2g_get_ackid(frame))
		{
			unsigned char rx_ackid = of2g_get_ackid(netman->current_rx_ack);
			// Received ack is not the same as the previously received ACK
			if(rx_ackid != of2g_get_ackid(frame))
			{
				netman->tx_state = NOT_WAITING_FOR_ACK;
				netman->rx_state = NEW_ACK;
		  	memcpy(&netman->current_rx_ack, frame, sizeof(of2g_frame_t));
			}
			else
			{
				netman->tx_state = WAITING_FOR_ACK;
			}
		}
	}
	else if (frametype == OF2G_DATA)
	{
		// Received frame is different from last received frame
		if(netman->current_rx_fid != of2g_get_fid(frame)) // TODO: what if our last received frame was an ack with FID same as this data frame?
		{
			netman->rx_state = NEW_DATA;
			netman->current_rx_fid = of2g_get_fid(frame);
 			memcpy(&netman->current_rx_data, frame, sizeof(of2g_frame_t));
			// Prepare ack frame to send based on received frame
			build_ack_frame(netman,frame);
		}
		else // TODO: verify case if fids dont match
		{
			if(of2g_get_length(frame) == of2g_get_length(netman->current_rx_data)) // same fid, but different contents
			{
				netman->rx_state = DUP_DATA;
			}
		}
	}

	else {
	}
}
