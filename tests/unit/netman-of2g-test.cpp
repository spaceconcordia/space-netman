// Using GoogleTests
#include "../../include/of2g.h"

class Netman_of2g_Test : public ::testing::Test {
  protected:
  virtual void SetUp() {
    valid_dataframe = { };
    valid_ackframe = { };
    unsigned char buffer[256] = { };
  }
  of2g_frame valid_dataframe;
  of2g_frame valid_ackframe;
  
};

// Check FID byte
TEST_F(Netman_of2g_Test, ValidFID) {
  // FID byte matches expected
  ASSERT_EQ(expected, valid_dataframe[0]);
  ASSERT_EQ(expected, valid_ackframe[0]);
}

// Check ACKID byte
TEST_F(Netman_of2g_Test, ValidACK) {
  // ACKID byte matches expected
  ASSERT_EQ(expected, valid_dataframe[1]);
  ASSERT_EQ(expected, valid_ackframe[1]);
}

// Check Length Byte
TEST_F(Netman_of2g_Test, ValidLength) {
  // verify length byte
  ASSERT_EQ(expected, valid_dataframe[2]);
  ASSERT_EQ(expected, valid_ackframe[2]);
}

// Check Checksum byte 
TEST_F(Netman_of2g_Test, GoodChecksum) {
  // verify check sum bytes
  HE100_checksum frame_checksum = HE100_fletcher16((char*)*valid_dataframe, length+3);
  ASSERT_EQ(*(valid_dataframe[length+3]), frame_checksum.sum1);
  ASSERT_EQ(*(valid_dataframe[3+length+1]), frame_checksum.sum2);
  
  // verify check sum bytes
  HE100_checksum frame_checksum = HE100_fletcher16((char*)*valid_ackframe, length+3);
  ASSERT_EQ(*(valid_ackframe[length+3]), frame_checksum.sum1);
  ASSERT_EQ(*(valid_ackframe[3+length+1]), frame_checksum.sum2);
  
}

// Check for correct data frame
TEST_F(Netman_of2g_Test, GoodDataFrame) {
  
  size_t length = 0;
  unsigned char fid = 0;
  of2g_frame_t * out;
  of2g_build_data_frame(buffer,length,fid,out);
  ASSERT_STREQ(expected_str, actual_str);	
 
}

// Check for correct ACk frame
TEST_F(Netman_of2g_Test, GoodAckFrame) {
  
  size_t length = 0;
  unsigned char fid = 0;
  of2g_frame_t * out;
  of2g_build_ack_frame(buffer,out);
  ASSERT_STREQ(expected_str, actual_str);	
 
}

// check for correct ack frame
TEST_F(Netman_of2g_Test, GoodAckFrame) {
  
  of2g_frame_t * out;
  of2g_build_ack_frame(buffer,out);
  ASSERT_STREQ(expected_str, actual_str);	
 
}
