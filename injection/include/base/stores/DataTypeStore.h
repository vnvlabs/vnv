
/**
  @file ActionStore.h
**/
#ifndef VV_DATATYPESTORE_HEADER
#define VV_DATATYPESTORE_HEADER

#include <map>
#include <string>

#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IDataType.h"
#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"

namespace VnV {


class DataTypeStore : public BaseStore {

  std::map<long long, dataType_ptr> dataType_factory;
 public:

  DataTypeStore(){}




   void addDataType(std::string packageName, std::string name, dataType_ptr m);

  IDataType_ptr getDataType(long long key);

  IDataType_ptr getDataType(std::string name);

  template <typename T> IDataType_ptr getDataType() {
    return getDataType(typeid(T).name());
  }

  void print();

  static DataTypeStore& instance();

};

}  // namespace VnV

#endif
