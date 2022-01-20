#include "dlclass1.h"
#include <string>

// Quick little dummy application showning how a final application can use a 
// third party application to init and finalize vnv. In this case, the user doesn't
// even need to link the VnV library. 

int main(int argc, char** argv) {
  std::string s = "input.json";
  DummyVnV::Initialize(argc,argv,s.c_str());
  dummyLibOne::class1 a;
  a.function1(3);
  DummyVnV::Finalize();

}
