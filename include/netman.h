#ifndef NETMAN_H_
#define NETMAN_H_

#include <inttypes.h>

#include "of2g.h"

// This enum will
typedef enum {
   NEW_ACK;
   NEW_DATA;
   DUP_DATA;
   BAD_CSUM;
   BAD_FID;
} netman_rx_state_t

typedef enum {
   WAITING_FOR_ACK;
   NOT_WAITING_FOR_ACK;
} netman_tx_state_t;

typedef struct {
   of2g_frame_t current_tx_ack;
   of2g_frame_t current_tx_data;

   of2g_frame_t current_rx_ack;
   of2g_frame_t current_rx_data;

   uint8_t next_rx_fid;
   uint8_t next_tx_fid;

   netman_tx_state_t tx_state;
   netman_rx_state_t rx_state;

}  netman_t;


// Should perform any and all necessary initialization of netman
// netman must be ready to use after this.
void netman_init(netman_t * netman);

// Build an of2g DATA frame based on the given buffer of raw data, and
// store it in netman->current_tx_data
void build_data_frame(netman_t * netman, uint8_t * buffer, uint8_t length);

// Build an of2g ACK frame based on the given data frame
void build_ack_frame (netman_t * netman, of2g_frame_t * data_frame);


#endif
