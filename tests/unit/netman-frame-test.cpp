// using GoogleTests
#include "gtest/gtest.h"
#include "../../include/of2g.h"
#include "../../include/netman.h"

class Netman_frame_Test : public ::testing::Test {
	protected:
	virtual void SetUp() {
// test for receiving
/*	
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
	*/
}
	
	
	of2g_frame_t valid_dataframe = { 0x73, 0x0, 0x09, 0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0x4c, 0xe8};
  of2g_frame_t expected_ackframe = { 0x01, 0x73, 0x0, 0x74, 0xe9};  
	unsigned char buffer[10] = {0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0x0};
	netman_t netman_rx;
	netman_t netman_tx;
};

// Check new data frame was built according to raw data and was stored correctly
TEST_F(Netman_frame_Test, GoodDataFrameToSend) {
	netman_tx.current_tx_fid = 0x73;
	size_t length = 9;
	printf("building from test\n");
	netman_new_tx_bytes(&netman_tx, buffer, length);
	// verify tx_state and current_tx_data was set
	ASSERT_STREQ(reinterpret_cast<const char*>(valid_dataframe),reinterpret_cast<const char*>(netman_tx.current_tx_data));


	ASSERT_EQ(WAITING_FOR_ACK, netman_tx.tx_state);
	 
	// verify fid was set
	ASSERT_EQ(0x73, netman_tx.current_tx_fid);

}

// Verify that Ack is sent based on data we receive
TEST_F(Netman_frame_Test, GoodAckFrameToSend) {
	
	memcpy(netman_tx.current_rx_data, valid_dataframe, sizeof(of2g_frame_t));
	build_ack_frame(&netman_tx, netman_tx.current_rx_data);
	// current FID changed?
	ASSERT_EQ(0x1, netman_tx.current_tx_fid);
	// current sent ACK changed?
//	ASSERT_STREQ(reinterpret_cast<const char*>(expected_ackframe), reinterpret_cast<const char*>(netman_tx.current_tx_ack));

	int i;
	size_t frame_length = of2g_get_frame_length(expected_ackframe);
	for(i = 0;i < frame_length; ++i)
		ASSERT_EQ(expected_ackframe[i], netman_tx.current_tx_ack[i]);


}
/*
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
