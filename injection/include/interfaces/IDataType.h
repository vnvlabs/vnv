#ifndef VNV_IDATATYPE_HEADER
#define VNV_IDATATYPE_HEADER

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common-interfaces/all.h"

namespace VnV {

class IOutputEngine;

class IDataType;
typedef std::shared_ptr<IDataType> IDataType_ptr;
typedef std::vector<IDataType_ptr> IDataType_vec;

enum class SupportedDataType { DOUBLE, LONG, STRING, JSON };

class PutData {
 public:
  std::string name;
  SupportedDataType datatype;
  std::vector<std::size_t> shape;
};

class IDataType {
 private:
  long long key;

 public:
  IDataType();
  virtual void setData(void* /*data*/);
  virtual void Put(VnV::IOutputEngine* /*engine*/) {}

  virtual std::map<std::string, PutData> getLocalPutData() { return {}; }

  virtual void* getPutData(std::string /*name*/);

  // Needed if you want to support data type redcution and vectors.
  virtual long long maxSize();

  virtual long long pack(void* /*buffer*/);

  virtual void unpack(void* /*buffer*/) ;

  // Needed if you want to use reduction operations.
  virtual void axpy(double /*alpha*/, IDataType_ptr /*y*/);
  
  virtual int compare(IDataType_ptr /*y*/) ;

  virtual void mult(IDataType_ptr /*y*/);

  virtual std::string typeId() ;
  virtual std::string displayName() ;

  void setKey(long long key);
  long long getKey();
  virtual ~IDataType();
};

typedef IDataType* (*dataType_ptr)();

void registerDataType(std::string packageName, std::string name, VnV::dataType_ptr r);

template <typename T> void registerDataType(std::string packageName, VnV::dataType_ptr ptr) {
  std::string s = typeid(T).name();
  VnV::registerDataType(packageName, s, ptr);
}

}  // namespace VnV

#define INJECTION_DATATYPE(PNAME, name, cls)                                              \
  namespace VnV {                                                                         \
  namespace PNAME {                                                                       \
  namespace DataTypes {                                                                   \
  VnV::IDataType* declare_##name();                                                       \
  void register_##name() { VnV::registerDataType<cls>(VNV_STR(PNAME), &declare_##name); } \
  }                                                                                       \
  }                                                                                       \
  }                                                                                       \
  VnV::IDataType* VnV::PNAME::DataTypes::declare_##name()

#define INJECTION_TDATATYPE(PNAME, cname, cls)                     \
  INJECTION_DATATYPE(PNAME, cname, cls) {                          \
    class datatype : public VnV::IDataType {                       \
      std::string typeId() override { return typeid(cls).name(); } \
      std::string displayName() override { return #cls; }          \
    };                                                             \
    return new datatype();                                         \
  }

#define DECLAREDATATYPE(PNAME, name) \
  namespace VnV {                    \
  namespace PNAME {                  \
  namespace DataTypes {              \
  void register_##name();            \
  }                                  \
  }                                  \
  }
#define REGISTERDATATYPE(PNAME, name) VnV::PNAME::DataTypes::register_##name();

#endif
