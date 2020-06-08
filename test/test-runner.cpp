

#include "VnV.h"
int main(int argc, char** argv) {
  INJECTION_INITIALIZE(&argc, &argv,
                       (argc == 2) ? argv[1] : "./test-config.json");
  VnV_runUnitTests(VnV_Comm_World);
  INJECTION_FINALIZE();
}
