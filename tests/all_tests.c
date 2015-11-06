#include "unity_fixture.h"

static void RunAllTests(void) {
  RUN_TEST_GROUP(TestDiskEmulator);
  RUN_TEST_GROUP(TestMkfs);
  RUN_TEST_GROUP(TestDataBlocksHandler);
  RUN_TEST_GROUP(TestInodesHandler);
}

int main(int argc, const char * argv[]) {
  return UnityMain(argc, argv, RunAllTests);
}
