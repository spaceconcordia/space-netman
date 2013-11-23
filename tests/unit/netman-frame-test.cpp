// using GoogleTests
#include "gtest/gtest.h"
#include "../../include/of2g.h"
#include "../../include/netman.h"

class Netman_frame_Test : public ::testing::Test {
	protected:
	virtual void SetUp() {
}
	
	
	of2g_frame_t valid_dataframe = { 0x01, 0x0, 0x09, 0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0xda, 0x90};
  of2g_frame_t expected_ackframe = { 0x01, 0x01, 0x0, 0x02, 0x05};  
	unsigned char buffer[10] = {0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0x0};
	netman_t netman;
	
};

// Verify that Ack is sent based on data we receive
TEST_F(Netman_frame_Test, GoodAckFrameToSend) {
	
	memcpy(netman.current_rx_data, valid_dataframe, sizeof(of2g_frame_t));
	build_ack_frame(&netman, netman.current_rx_data);
	ASSERT_EQ(0x1, netman.current_tx_fid);

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

// Check new data frame was built according to raw data and was stored correctly
TEST_F(Netman_frame_Test, GoodDataFrameToSend) {
	netman.current_tx_fid = 0x01; 
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

// Test for received ack frame from a frame we sent
TEST_F(Netman_frame_Test, GoodAckFrameRead) {
	ASSERT_EQ(WAITING_FOR_ACK, netman.tx_state);
	netman_rx_frame(&netman, expected_ackframe);
	ASSERT_EQ(NOT_WAITING_FOR_ACK, netman.tx_state);
	ASSERT_EQ(OF2G_ACK, of2g_get_frametype(expected_ackframe));
	ASSERT_EQ(NEW_ACK, netman.rx_state);
	ASSERT_EQ(0x01, netman.current_tx_fid);
	size_t i;
	size_t frame_length = 5;
	for(i=0;i<frame_length;++i)
		ASSERT_EQ(expected_ackframe[i], netman.current_rx_ack[i]);
}

// verify if were still waiting for a new ack when we don't get the expected ack
TEST_F(Netman_frame_Test, DupAckFrameRead) {

	netman.tx_state = WAITING_FOR_ACK;
	netman_rx_frame(&netman, expected_ackframe);
	ASSERT_EQ(WAITING_FOR_ACK, netman.tx_state);

}
