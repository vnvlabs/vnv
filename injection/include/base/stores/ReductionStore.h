﻿
/**
  @file ActionStore.h
**/
#ifndef VV_REDUCTIONSTORE_HEADER
#define VV_REDUCTIONSTORE_HEADER

#include <map>
#include <string>

#include "base/stores/BaseStore.h"
#include "interfaces/IReduction.h"

namespace VnV {

class ReductionStore : public BaseStore {
  std::map<long long, reduction_ptr> reduction_factory;

 public:
  ReductionStore() {}

  IReduction_ptr getReducer(long long key);

  IReduction_ptr getReducer(std::string packageName, std::string name);

  IReduction_ptr getReducer(std::string packageColonName);

  void addReduction(std::string packageName, std::string name, reduction_ptr m);

  void print();

  static ReductionStore& instance();
};

}  // namespace VnV

#endif
