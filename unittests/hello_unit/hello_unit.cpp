#include "gtest/gtest.h"

// If we need to include c headers.
// Wrap the header in extern "C" so the c++ file can find the
// C library symbols to link to
/*extern "C" {
#include "pool/intpool.h"
}*/

TEST(hello_unit, test1) {
    EXPECT_EQ(1, 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
