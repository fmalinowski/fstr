#include "unity_fixture.h"

static void RunAllTests(void) {
  RUN_TEST_GROUP(TestDiskEmulator);
  RUN_TEST_GROUP(TestMkfs);
  RUN_TEST_GROUP(TestCommon);
  RUN_TEST_GROUP(TestDataBlocksHandler);
  RUN_TEST_GROUP(TestInodesHandler);
  RUN_TEST_GROUP(TestNameI);
  RUN_TEST_GROUP(TestSyscalls1);
  RUN_TEST_GROUP(TestSyscalls2);
}

int main(int argc, const char * argv[]) {
  return UnityMain(argc, argv, RunAllTests);
}
