
/**
  @file ActionStore.h
**/
#ifndef VV_DATATYPESTORE_HEADER
#define VV_DATATYPESTORE_HEADER

#include <map>
#include <string>

#include "common-interfaces/all.h"
#include "base/stores/BaseStore.h"
#include "interfaces/IDataType.h"
namespace VnV
{

  class DataTypeStore : public BaseStore
  {
    std::map<long long, dataType_ptr> dataType_factory;
    std::map<std::string, std::string> typeId_map;

  public:
    DataTypeStore() {}

    void addDataType(std::string packageName, std::string name, dataType_ptr m);

    IDataType_ptr getDataType(long long key);

    IDataType_ptr getDataType(std::string name);

    template <typename T>
    IDataType_ptr getDataType() { return getDataType(typeid(T).name()); }

    void print();

    static DataTypeStore &instance();
  };

} // namespace VnV

#endif
