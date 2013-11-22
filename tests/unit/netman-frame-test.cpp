// using GoogleTests
#include "gtest/gtest.h"
#include "../../include/of2g.h"
#include "../../include/netman.h"

class Netman_frame_Test : public ::testing::Test {
	protected:
	virtual void SetUp() {
// test for receiving
/*	
	netman->tx_state = WAITING_FOR_ACK;
	netman->rx_state = NEW_DATA;

	netman->current_tx_data = {};
	netman->current_tx_ack = {};
	netman->current_tx_fid = 0x02;

	netman->current_rx_data = {};
	netman->current_rx_ack = {};
	netman->current_rx_fid = 0x01;
// test for sending
	netman->tx_state = NOT_WAITING_FOR_ACK;
	netman->rx_state = DUP_DATA;
	
	netman->current_tx_data = {};
	netman->current_tx_ack = {};
	netman->current_tx_fid = 0x03;
	
	netman->current_rx_data = {};
	netman->current_rx_ack = {};
	netman->current_rx_fid = 0x02;
	*/
}

	of2g_frame_t valid_dataframe = { 0x01, 0x0, 0x09, 0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0xda, 0x90};
	of2g_frame_t expected_ackframe = { 0x01, 0x01, 0x0, 0x02, 0x05};

//	of2g_frame_t valid_dataframe = { 0x73, 0x0, 0x09, 0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0x4c, 0xe8};
//  of2g_frame_t expected_ackframe = { 0x01, 0x73, 0x0, 0x74, 0xe9};  
	unsigned char buffer[10] = {0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0x0};
	netman_t netman;
	
};

// Check new data frame was built according to raw data and was stored correctly
TEST_F(Netman_frame_Test, GoodDataFrameToSend) {
	netman.current_tx_fid = 0x01; // how is this normally set?
	size_t length = 9;
	netman_new_tx_bytes(&netman, buffer, length);
	// verify tx_state and current_tx_data was set
	size_t i;
	for(i=0;i<length+5;++i)
		ASSERT_EQ(valid_dataframe[i],netman.current_tx_data[i]);


	ASSERT_EQ(WAITING_FOR_ACK, netman.tx_state);
	 
	// verify fid was set
	ASSERT_EQ(0x01, netman.current_tx_fid);

}

// Verify that Ack is sent based on data we receive
TEST_F(Netman_frame_Test, GoodAckFrameToSend) {
	
	memcpy(netman.current_rx_data, valid_dataframe, sizeof(of2g_frame_t));
	build_ack_frame(&netman, netman.current_rx_data);
	// current FID changed?
	ASSERT_EQ(0x1, netman.current_tx_fid);
	// current sent ACK changed?

	size_t i;
	size_t frame_length = of2g_get_frame_length(expected_ackframe);
	for(i = 0;i < frame_length; ++i)
		ASSERT_EQ(expected_ackframe[i], netman.current_tx_ack[i]);
}

// Verify reception of data frame
TEST_F(Netman_frame_Test, GoodDataFrameRead) { 
	//  DATA: New data (ACK SENT?)
  netman_rx_frame(&netman, valid_dataframe);	
  ASSERT_EQ(NEW_DATA, netman.rx_state);
  // test memcpy ( read frame with current_rx_data )
	size_t frame_length = valid_dataframe[2] + 5;
	size_t i;
	for(i = 0;i < frame_length; ++i)
		ASSERT_EQ(valid_dataframe[i], netman.current_rx_data[i]);
}

TEST_F(Netman_frame_Test, GoodAckFrameRead) {
 // ACK: duplicate ack, new ack
	ASSERT_EQ(OF2G_ACK, of2g_get_frametype(expected_ackframe));
	netman_rx_frame(&netman, expected_ackframe);
	ASSERT_EQ(0x01, netman.current_tx_fid);
	ASSERT_EQ(NEW_ACK, netman.rx_state);
	// test memcpy
	size_t i;
	size_t frame_length = 5;
	for(i=0;i<frame_length;++i)
		ASSERT_EQ(expected_ackframe[i], netman.current_rx_ack[i]);
}

