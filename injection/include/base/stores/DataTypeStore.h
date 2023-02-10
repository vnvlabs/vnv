
/**
  @file ActionStore.h
**/
#ifndef VV_DATATYPESTORE_HEADER
#define VV_DATATYPESTORE_HEADER

#include <map>
#include <string>

#include "base/Utilities.h"
#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
#include "common-interfaces/Communication.h"
#include "common-interfaces/PackageName.h"
#include "interfaces/IDataType.h"
namespace VnV {

class DataTypeStore : public BaseStore {
  std::map<long long, dataType_ptr> dataType_factory;
  std::map<std::string, std::string> typeId_map;
  bool type_map_initialized = false;

 public:
  DataTypeStore() {}

  void addDataType(std::string packageName, std::string name, dataType_ptr m);

  IDataType_ptr getDataType(long long key);

  IDataType_ptr getDataType(std::string name);

  template <typename T> IDataType_ptr getDataType() { return getDataType(typeid(T).name()); }

  void print();

  void initializeMap() {
    if (type_map_initialized == false) {
      for (auto& it : dataType_factory) {
        auto d = it.second();
        typeId_map[d->typeId()] = d->displayName();
      }
      type_map_initialized = true;
    }
  }

  static DataTypeStore& instance();
};

}  // namespace VnV

#endif
