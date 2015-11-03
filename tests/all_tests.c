#include "unity_fixture.h"

static void RunAllTests(void) {
  RUN_TEST_GROUP(TestDiskEmulator);
  RUN_TEST_GROUP(TestBlocksHandler);
}

int main(int argc, const char * argv[]) {
  return UnityMain(argc, argv, RunAllTests);
}
