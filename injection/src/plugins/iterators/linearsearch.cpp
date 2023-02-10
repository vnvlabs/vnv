

#include <iostream>

#include "interfaces/IIterator.h"

namespace {
static std::string default_message = "Here";
static std::string default_type = "Put";
}  // namespace

template <typename T> class SingleVariableMinimization {
 public:
  T min;
  T max;
  T step;
  T xoptimal;
  T yoptimal;
  int count = 0;
  bool config = false;

  bool configured() { return config; }

  void setRange(T min_, T max_, T step_) {
    min = min_;
    max = max_;
    step = step_;
    config = true;
  }

  int iterate(T* next, T* feval) {
    if (feval == nullptr) {
      *next = min;
      return true;
    } else if (count == 0 || *feval < yoptimal) {
      xoptimal = min + count * step;
      yoptimal = *feval;
    }
    count += 1;
    *next = min + count * step;
    return (*next <= max);
  }
};
template class SingleVariableMinimization<double>;

/**

   VnV Linear parameter search
   ===========================

   The optimal values for this function are [:vnv:`data.xopt` , :vnv:`data.yopt`].

**/
INJECTION_ITERATOR_R(VNVPACKAGENAME, singleParameterMinimization, SingleVariableMinimization<double>) {
  const json& p = getConfigurationJson();
  if (!runner->configured()) {
    runner->setRange(p["min"].get<double>(), p["max"].get<double>(), p["step"].get<double>());
  }
  double* next = getInputPtr<double>(p["variable"], "double");
  double* feval = getInputPtr<double>(p["feval"], "double");
  int more = runner->iterate(next, feval);
  if (!more) {
    engine->Put("xopt", runner->xoptimal);
    engine->Put("yopt", runner->yoptimal);
  }
  return more;
}
