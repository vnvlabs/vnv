
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

// Actions are parameterless functions that can be executed at runtime by the
// user through the input file. There are designed to support little stand along
// tasks like writing help messages.
INJECTION_ACTION(Samples, sampleAction) {
  std::cout << "This is a help message" << std::endl;
}
