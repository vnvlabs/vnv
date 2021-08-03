
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
namespace VnV {


class ReductionStore {
 private:

  std::map<long long, reduction_ptr> reduction_factory;

  ReductionStore(){}


 public:

  static ReductionStore& instance();

  IReduction_ptr getReducer(long long key);

  IReduction_ptr getReducer(std::string packageName, std::string name);

  IReduction_ptr getReducer(std::string packageColonName);

  void addReduction(std::string packageName, std::string name, reduction_ptr m);

  void print();
};

}  // namespace VnV

#endif
