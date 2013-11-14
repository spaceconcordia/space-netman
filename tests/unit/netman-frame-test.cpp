// using GoogleTests
#include "../../of2g.h"
#include "../../netman.h"

class Netman_frame_Test : public ::testing::Test {
	  protected:
			virtual void SetUp() {
				 valid_dataframe = { };
				 valid_ackframe = { };
				 badchecksum_frame = { };
				 unsigned char buffer[256] = { };
				 netman_init(&netman);
				}

				 of2g_frame valid_dataframe;
			   of2g_frame valid_ackframe;
				 of2g_frame badchecksum_frame;
				 netman_t * netman;
};

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
	  
	  build_ack_frame (netman, netman->current_rx_data);
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

TEST_F(Netman_frame_Test, GoodAckFrameRead) {
	 // ACK: duplicate ack, new ack
		netman_rx_frame(netman, valid_ackframe);
		ASSERT_EQ(expected, netman->rx_state);
		// test memcpy
		ASSERT_STREQ(expected_str, netman->current_rx_ack);
}

