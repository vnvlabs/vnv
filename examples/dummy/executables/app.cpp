#include "dlclass1.h"
#include <string>
// Quick little dummy application showning how a final application can use a 
// third party application to init and finalize vnv. In this case, the user doesn't
// even need to link the VnV library. 

int main(int argc, char** argv) {
  std::string s = "vv-input.json";
  DummyVnV::Initialize(argc,argv,s.c_str());
  int x = 10;
  DummyVnV::Finalize();

}
