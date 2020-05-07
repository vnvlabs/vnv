
#include <iostream>
#include <vector>

#include "VnV.h"


void callback() {
}

int main(int argc, char** argv) {

  VnV_init(&argc, &argv, (argc==2) ? argv[1] : "./vv-input.json", callback);
  VnV_readFile("./vv-output.json");
  VnV_finalize();
}
