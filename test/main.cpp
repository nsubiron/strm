#include "util/profiler.h"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::util::profiler::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
