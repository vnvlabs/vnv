

#include "VnV.h"

#define SPNAME Template


INJECTION_TEST(SPNAME,trial) {
  //dumpParameters();
  return SUCCESS;
}

namespace fff {


class ff{
  public:
  

  template <typename T, typename X> int ctemplateFnc(int x, T y, X xx) {
  INJECTION_POINT(VNV_STR(SPNAME), VSELF, "ctemplateFn", x, y, xx);
  return 0;
}

};




template<typename G>
class fft {
public:
  G xxx;
  template <typename T, typename X> int tctemplateFnc(int x, T y, X xx) {
    INJECTION_POINT(VNV_STR(SPNAME), VSELF, "tctemplateFn",   x, y, xx);
    return 0;
  }
};







// Template Function
template <typename A, typename B, typename C, typename D> int templateFnc(A cls, B template_cls, C vec_cls, D vec_template_cls) {
  INJECTION_POINT(VNV_STR(SPNAME), VSELF, "templateFn", cls, template_cls, vec_cls, vec_template_cls);
  return 0;
}



// Template Function
template <typename E> int templateFnc() {
  std::vector<E> vec_tmp;
  std::vector<std::vector<E>> vec_vec_tmp;

  INJECTION_POINT(VNV_STR(SPNAME), VSELF, "templateFn1", vec_tmp, vec_vec_tmp );
  return 0;
}

}

INJECTION_EXECUTABLE(SPNAME)

int main(int argc, char** argv) {
  
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, (argc == 2) ? argv[1] : "./vv-input.json");

  fff::ff cls;    
  fff::fft<fff::ff> tcls;
  std::vector<fff::ff> vcls;
  std::vector<fff::fft<fff::ff>> vtcls;
  fff::templateFnc(cls, tcls, vcls, vtcls);
  fff::templateFnc( tcls, cls, cls, tcls);


  fff::templateFnc<double>();
  tcls.tctemplateFnc(1,cls,vcls);


}

  

