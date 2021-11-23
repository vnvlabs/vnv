
/**
  @file ActionStore.h
**/
#ifndef VV_DATATYPESTORE_HEADER
#define VV_DATATYPESTORE_HEADER

#include <map>
#include <string>

#include "base/parser/JsonParser.h"
#include "base/stores/BaseStore.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/IDataType.h"
#include "base/Utilities.h"
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

  template <typename T> IDataType_ptr getDataType() {
    return getDataType(typeid(T).name());
  }

  void print();

  

  template<typename ...Args> 
  std::map<std::string,std::string> getDataTypeMap(std::string pnames) {
      std::vector<std::string> result;
      VnV::StringUtils::StringSplit(pnames,",", result);
      return getDataTypeMap<Args...>(result);
  }

  template<typename A> 
  std::map<std::string,std::string> getDataTypeMap(std::vector<std::string> pnames) {
     assert(pnames.size()==0);
     std::map<std::string, std::string> r;
     r[pnames[0]] = getDataTypeEntry<A>();
     return r;
  }

  template<typename A, typename U, typename ...Args> 
  std::map<std::string,std::string> getDataTypeMap(std::vector<std::string> pnames) {
     std::string s = pnames[0];
     pnames.erase(pnames.begin());
     auto a = getDataTypeMap<U, Args...>(pnames);
     a[s] = getDataTypeEntry<A>();
     return a;
  }

  template<typename T>
  std::string getDataTypeEntry() {
     initializeMap(); 
     auto it = typeId_map.find(typeid(T).name());
     if (it != typeId_map.end()) {
       return it->second;
     }
     throw VnVExceptionBase("Unknown Data Type");
  
  }

  void initializeMap() {     
     if (type_map_initialized == false) {
        for (auto &it : dataType_factory) {
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
