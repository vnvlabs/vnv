
/**
  @file InjectionPoint.h
**/
#ifndef VV_ITERATION_POINTS_H
#define VV_ITERATION_POINTS_H


#include "base/points/InjectionPoint.h"
#include "interfaces/IIterator.h"

namespace VnV {

class IterationPoint : public InjectionPoint {
  std::vector<std::shared_ptr<IIterator>> m_iterators;
  int started = 0;
  int once = 0;
public:
  IterationPoint(std::string packageName, std::string name, json registrationJson, int once_, NTV& in_args, const NTV& out_args) 
  : InjectionPoint(packageName, name, registrationJson, in_args, out_args), once(once_) {};
  void addIterator(IteratorConfig &c);
  virtual bool iterate(std::string function, int line);
};

}  // namespace VnV

#endif  // include gaurd.
