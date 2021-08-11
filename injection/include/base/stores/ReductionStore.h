
/**
  @file ActionStore.h
**/
#ifndef VV_REDUCTIONSTORE_HEADER
#define VV_REDUCTIONSTORE_HEADER

#include <map>
#include <string>

#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IReduction.h"
#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"

namespace VnV {


class ReductionStore : public BaseStore{

  std::map<long long, reduction_ptr> reduction_factory;

 public:
  ReductionStore(){}


  IReduction_ptr getReducer(long long key);

  IReduction_ptr getReducer(std::string packageName, std::string name);

  IReduction_ptr getReducer(std::string packageColonName);

  void addReduction(std::string packageName, std::string name, reduction_ptr m);

  void print();

  static ReductionStore& instance();

};

}  // namespace VnV

#endif
