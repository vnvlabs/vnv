﻿#include <iostream>

#include "base/stores/ReductionStore.h"
#include "base/Utilities.h"
#include "base/exceptions.h"



namespace VnV {

namespace { 
  
long long getKey(std::string packageName,
                                     std::string name) {
  return VnV::StringUtils::simpleHash(packageName + ":" + name);
}

long long getKey(std::string name) {
  return getKey("", name);
}

}

void ReductionStore::addReduction(std::string packageName, std::string name,
                                      reduction_ptr m) {
  reduction_factory.insert(std::make_pair(getKey(packageName, name), m));
}

IReduction_ptr ReductionStore::getReducer(long long key) {
  auto it = reduction_factory.find(key);
  if (it != reduction_factory.end()) {
    IReduction_ptr s(it->second());
    s->setKey(key);
    return s;
  }
  throw VnV::VnVExceptionBase("Un supported Data Type)");
}

IReduction_ptr ReductionStore::getReducer(std::string packageName,
                                              std::string name) {
  return getReducer(getKey(packageName, name));
}

IReduction_ptr ReductionStore::getReducer(std::string packageColonName) {
  return getReducer(VnV::StringUtils::simpleHash(packageColonName));
}


ReductionStore& ReductionStore::instance() {
  static ReductionStore store;
  return store;
}


}  // namespace VnV
