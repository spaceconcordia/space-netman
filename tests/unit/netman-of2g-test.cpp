
// Using GoogleTests
#include "gtest/gtest.h"
#include "../../include/of2g.h"

class Netman_of2g_Test : public ::testing::Test {
  protected:
  virtual void SetUp() {

  }
  of2g_frame_t valid_dataframe = { 0x73, 0x0, 0x09, 0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63, 0x4c, 0xe8};
  unsigned char buffer[256] = {0x6f, 0x6c, 0x69, 0x76, 0x69, 0x65, 0x72, 0x73, 0x63};
  
};

// Check Checksum byte
TEST_F(Netman_of2g_Test, GoodChecksum) {
  // verify check sum bytes
  ASSERT_EQ(1, of2g_valid_frame(valid_dataframe));

}

// Check FID byte
TEST_F(Netman_of2g_Test, ValidFID) {
  // FID byte matches expected
  ASSERT_EQ(0x73, of2g_get_fid(valid_dataframe));
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
  unsigned char fid = 0x73;
  of2g_frame_t out;
  ASSERT_EQ(1, of2g_build_data_frame(buffer,length,fid,out));
  ASSERT_EQ(valid_dataframe, out);
 
}
/*

// Check for correct ACk frame built
TEST_F(Netman_of2g_Test, GoodAckFrame) {
  
  unsigned char fid = 0x01;
  of2g_frame_t * out;
  ASSERT_EQ(1, of2g_build_ack_frame(buffer,out));
  ASSERT_STREQ(valid_ackframe, out);
 
}
*/
