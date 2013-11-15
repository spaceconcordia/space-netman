#ifndef NETMAN_H_
#define NETMAN_H_

// This file handles the rx and tx state of the OF2G protocol.
//
// It determines the appropriate RX and TX actions based on sent
// and received frames. It has no conception of either IO or timing
// simply shovelling frames around.


#include <inttypes.h>
#include <string.h>
#include "of2g.h"

// This enum describes the most recently received frame
typedef enum {
   NEW_ACK,
   NEW_DATA,
   DUP_DATA,
   BAD_CSUM,
   BAD_FID
} netman_rx_state_t;

// this enum describes the status of the most recently sent
// DATA frame. (N.B., we don't care about our sent ACKs really.
// If we don't hear anything back we assume they are received.
// If they're not received, we will be notified).
typedef enum {
   WAITING_FOR_ACK,
   NOT_WAITING_FOR_ACK
} netman_tx_state_t;

typedef struct {

   // These members both refer to the last sent DATA. `current_tx_data`
   // is the data frame itself, `current_tx_fid` is the frame id of
   // `current_tx_data`.
   unsigned char current_tx_fid;
   of2g_frame_t current_tx_data;

   // This member holds the last transmitted ACK. This frame is always the
   // most recent response to `current_rx_data`
   of2g_frame_t current_tx_ack;

   // These members both refer to the last received DATA. `current_rx_fid` is
   // the frame id of `current_rx_data`, which is the most recently received
   // valid data.
   unsigned char      current_rx_fid;
   of2g_frame_t current_rx_data;

   // This member holds the last received valid ACK. This frame should be a
   // response to a DATA that we sent.
   of2g_frame_t current_rx_ack;

   // These members keep track of the states of the rx and tx channels.
   // descriptions of their values are in the enum definitions above.
   netman_tx_state_t tx_state;
   netman_rx_state_t rx_state;

}  netman_t;

// Should perform any and all necessary initialization of netman
// netman must be ready to use after this.
void netman_init(netman_t * netman);

// Build an of2g DATA frame based on the given buffer of raw data, and
// store it in netman->current_tx_data
void netman_new_tx_bytes(netman_t * netman, unsigned char * buffer, size_t length);

// Build an of2g ACK frame based on the given data frame
void build_ack_frame (netman_t * netman, of2g_frame_t data_frame);


// This function should handle the received `frame`, changing the state 
// of `netman` and updating `current_*x_*` members appropriately
void netman_rx_frame(netman_t * netman, of2g_frame_t frame);


#endif
