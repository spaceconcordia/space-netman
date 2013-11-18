// using GoogleTests
#include "gtest/gtest.h"
#include "../../include/of2g.h"
#include "../../include/netman.h"

class Netman_frame_Test : public ::testing::Test {
	protected:
	virtual void SetUp() {
// test for receiving
	netman_rx->tx_state = WAITING_FOR_ACK;
	netman_tx->rx_state = NEW_DATA;

	netman_rx->current_tx_data = {};
	netman_rx->current_tx_ack = {};
	netman_rx->current_tx_fid = 0x02;

	netman_rx->current_rx_data = {};
	netman_rx->current_rx_ack = {};
	netman_rx->current_rx_fid = 0x01;
// test for sending
	netman_tx->tx_state = NOT_WAITING_FOR_ACK;
	netman_tx->rx_state = DUP_DATA;
	
	netman_tx->current_tx_data = {};
	netman_tx->current_tx_ack = {};
	netman_tx->current_tx_fid = 0x03;
	
	netman_tx->current_rx_data = {};
	netman_tx->current_rx_ack = {};
	netman_tx->current_rx_fid = 0x02;
	
}
	
	
	of2g_frame_t valid_dataframe = {};
	unsigned char * buffer;
	netman_t * netman_rx;
	netman_t * netman_tx;
};
/*
// Check new data frame was built according to raw data and was stored correctly
TEST_F(Netman_frame_Test, GoodDataFrameToSend) {
	netman_new_tx_bytes(netman, buffer, length);
	// verify tx_state and current_tx_data was set
	ASSERT_STREQ(expected_str, netman->current_tx_data);
	ASSERT_EQ(expected, netman->tx_state);
	 
	// verify new fid
	ASSERT_EQ(expected, netman->current_tx_fid);
}

// Verify that Ack is sent based on data we receive
TEST_F(Netman_frame_Test, GoodAckFrameToSend) {
	  
	build_ack_frame(netman, netman->current_rx_data);
	// current FID changed?
	ASSERT_EQ(expected, netman->current_tx_fid);
	// current sent ACK changed?
	ASSERT_EQ(expected, netman->current_tx_ack);
	// check current_tx_ack
	ASSERT_STREQ(expected_str, netman->current_tx_data); 
}

// Verify reception of data frame
TEST_F(Netman_frame_Test, GoodDataFrameRead) { 
	//  DATA: New data (ACK SENT?)
  netman_rx_frame(netman, valid_dataframe);	
  ASSERT_EQ(expected, netman->rx_state);
	//  duplicate data, bad data 
  ASSERT_EQ(expected, netman->rx_state);
	// NEITHER ACK OR DATA 
  ASSERT_EQ(expected, netman->rx_state);
	//  bad checksum
  ASSERT_EQ(expected, netman->rx_state);
  // test memcpy ( read frame with current_rx_data )
	ASSERT_STREQ(expected_str, netman->current_rx_data);
}


TEST_F(Netman_frame_Test, DupDataFrameRead) {


}


TEST_F(Netman_frame_Test, GoodAckFrameRead) {
 // ACK: duplicate ack, new ack
	netman_rx_frame(netman, valid_ackframe);
	ASSERT_EQ(expected, netman->rx_state);
	// test memcpy
	ASSERT_STREQ(expected_str, netman->current_rx_ack);
}
*/
