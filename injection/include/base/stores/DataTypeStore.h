
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
namespace VnV {


class DataTypeStore {
 private:
  std::map<long long, dataType_ptr> dataType_factory;

  DataTypeStore(){}


 public:

  static DataTypeStore& instance();


   void addDataType(std::string packageName, std::string name, dataType_ptr m);

  IDataType_ptr getDataType(long long key);

  IDataType_ptr getDataType(std::string name);

  template <typename T> IDataType_ptr getDataType() {
    return getDataType(typeid(T).name());
  }

  void print();
};

}  // namespace VnV

#endif
