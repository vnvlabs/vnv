
/**
  @file InjectionPoint.h
**/
#ifndef VV_PLUG_POINTS_H
#define VV_PLUG_POINTS_H

#include "base/points/InjectionPoint.h"
#include "interfaces/IPlug.h"

namespace VnV {

class PlugPoint : public InjectionPoint {
  std::shared_ptr<IPlug> m_plugs;
  bool started = false;

 public:
  PlugPoint(std::string packageName, std::string name, json registrationJson,
            NTV& in_args, const NTV& out_args)
      : InjectionPoint(packageName, name, registrationJson, in_args,
                       out_args){};

  virtual void setPlug(PlugConfig& config);
  virtual bool plug(std::string function, int line);
};

}  // namespace VnV

#endif  // include gaurd.
