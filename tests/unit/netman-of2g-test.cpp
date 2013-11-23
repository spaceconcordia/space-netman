
// Using GoogleTests
#include "gtest/gtest.h"
#include "../../include/of2g.h"

class Netman_of2g_Test : public ::testing::Test {
  protected:
  virtual void SetUp() {

  }
  of2g_frame_t valid_dataframe = { 0x01, 0x0, 0x09, 0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0xda, 0x90};
  of2g_frame_t expected_ackframe = { 0x01, 0x01, 0x0, 0x02, 0x05};
  unsigned char buffer[256] = {0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63};
  
};

// Check Checksum byte
TEST_F(Netman_of2g_Test, GoodChecksum) {
  // verify check sum bytes
  ASSERT_TRUE(of2g_valid_frame(valid_dataframe));
  ASSERT_TRUE(of2g_valid_frame(expected_ackframe));
}

// Check FID byte
TEST_F(Netman_of2g_Test, ValidFID) {
  // FID byte matches expected
  ASSERT_EQ(0x01, of2g_get_fid(valid_dataframe));
}



// Check ACKID byte
TEST_F(Netman_of2g_Test, ValidACK) {
  // ACKID byte matches expected
  ASSERT_EQ(0x0, of2g_get_ackid(valid_dataframe));
}


// Check Length Byte
TEST_F(Netman_of2g_Test, ValidDataLength) {
  // verify length byte
  ASSERT_EQ(9, of2g_get_length(valid_dataframe));
}

TEST_F(Netman_of2g_Test, ValidFrameLength) {
	// verify entire frame length, including header, data and checksum
  ASSERT_EQ(14, of2g_get_frame_length(valid_dataframe));
}

// Check for correct data frame
TEST_F(Netman_of2g_Test, GoodDataFrame) {
  
  size_t length = 9;
  unsigned char fid = 0x01;
	
  of2g_frame_t resulting_dataframe;
  ASSERT_TRUE(of2g_build_data_frame(buffer,length,fid,resulting_dataframe));

	size_t i;
	for(i = 0;i < length + 5; i++)
		ASSERT_EQ(valid_dataframe[i], resulting_dataframe[i]);
}


// Check for correct ACk frame built
TEST_F(Netman_of2g_Test, GoodAckFrame) {
  
  of2g_frame_t resulting_ackframe;
  ASSERT_TRUE(of2g_build_ack_frame(valid_dataframe,resulting_ackframe));
//  ASSERT_STREQ(reinterpret_cast<const char*>(expected_ackframe), reinterpret_cast<const char*>(resulting_ackframe));
  size_t frame_length = of2g_get_frame_length(expected_ackframe);
	size_t i;
	for(i=0;i<frame_length;i++)
		ASSERT_EQ(expected_ackframe[i],resulting_ackframe[i]);
}

// Check for returning frame type
TEST_F(Netman_of2g_Test, GoodFrameType) {
  
  ASSERT_EQ(OF2G_DATA, of2g_get_frametype(valid_dataframe));
  ASSERT_EQ(OF2G_ACK, of2g_get_frametype(expected_ackframe)); 
}

// check for correct data content
TEST_F(Netman_of2g_Test, GetGoodContent) {

  unsigned char out[9];
  size_t data_length = of2g_get_data_content(valid_dataframe, out);
  ASSERT_EQ(data_length, of2g_get_length(valid_dataframe));

  size_t i = 0;
//  printf("out = %s\nbuffer = %s \nlength: %d\n ", out, buffer, strlen(reinterpret_cast<const char*>(out)));
  for(i = 0; i < data_length; i++)
	ASSERT_EQ(buffer[i], out[i]);

 
}
