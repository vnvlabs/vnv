#include "common-interfaces/prettyfunction.h"

struct VnV_Function_Sig _VnV_function_sig(const char* compiler, const char* sig) {
  return (struct VnV_Function_Sig){compiler, sig};
}
