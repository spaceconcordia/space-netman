#include "gtest/gtest.h"
#include "netman-of2g-test.cpp"
#include "netman-frame-test.cpp"

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
