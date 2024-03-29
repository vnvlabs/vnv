﻿
/**
  @file InjectionPoint.h
**/
#ifndef VV_PLUG_POINTS_H
#define VV_PLUG_POINTS_H

#include "base/points/InjectionPoint.h"
#include "interfaces/IPlug.h"

namespace VnV {

class PlugPoint : public InjectionPoint {
  std::shared_ptr<IPlug> m_plug;
  bool started = false;

 public:
  PlugPoint(std::string packageName, std::string name, std::map<std::string, std::string> registrationJson, NTV& args)
      : InjectionPoint(packageName, name, registrationJson, args){};

  virtual void setPlug(PlugConfig& config);
  virtual bool plug(std::string function, int line, const DataCallback& callback);
};

}  // namespace VnV

#endif  // include gaurd.
