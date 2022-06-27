#ifndef VNV_IDATATYPE_HEADER
#define VNV_IDATATYPE_HEADER

#include "base/exceptions.h"
#include "common-interfaces/Logging.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

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
  virtual void setData(void* /*data*/) {
    HTHROW INJECTION_EXCEPTION("DataType %d does not support function setData", key);
  }
  // set from a raw pointer to the data.

  // Put will be called when you need to "Write" this output to file.
  // Put is called when someone type Put("name","datatype"). This is
  // collective on the Communicator used to type Put. So, this function
  // can output things like global vectors.
  // Put should call other Put actions
  virtual void Put(VnV::IOutputEngine* /*engine*/) {}

  // getLocalPutData is called when someone tries to write
  // a global vector containing objects of the this datatype
  // This is a "local" operation across the communicator set within
  // the engine.

  // This should be used to support writing vectors of a datatype. I.e., a
  // double vector. The Put is used to
  // support writing a (potentially distributed) data structure to file. This
  // function is used to support writing a distributed vector containing the
  // data structure to file.

  // The function should return a map of PutData objects that describe the
  // different parameters in the data structure. THIS MUST BE CONSTANT ACROSS
  // ALL INSTANCES OF THE DATA TYPE!!!!!!!!!!!!!!!!!!!!!!!!!!!!! WE DO NOT
  // SUPPORT DIFFERENT SIZES
  virtual std::map<std::string, PutData> getLocalPutData() { return {}; }

  // VnV will call this function when writing a distributed vector to file that
  // contains this data structure. The parameter name will be one of the keys
  // returned in the getLocalPutData map. The function should return a pointer
  // to the data associated with this key.
  virtual void* getPutData(std::string /*name*/) {
     HTHROW INJECTION_EXCEPTION("DataType %d does not support function getPutData", key);
  }

  // Needed if you want to support data type redcution and vectors.
  virtual long long maxSize() {
      throw INJECTION_EXCEPTION("DataType %d does not support function maxSize", key);
  }  // what is the maximum size of the buffer
  
  virtual long long pack(void* /*buffer*/) {
     throw INJECTION_EXCEPTION("DataType %d does not support function pack", key);
  }  // pack the buffer
  
  virtual void unpack(void* /*buffer*/) {
   throw INJECTION_EXCEPTION("DataType %d does not support function unpack", key);
  }  // unpack into a buffer

  // Needed if you want to use reduction operations.
  virtual void axpy(double /*alpha*/, IDataType_ptr /*y*/) {
      throw INJECTION_EXCEPTION("DataType %d does not support function axpy", key);
  }  // y = ax + y
  
  virtual int compare(IDataType_ptr /*y*/) {
     throw INJECTION_EXCEPTION("DataType %d does not support function compare", key);
  }  // -1 less, 0 == , 1 greater.
  
  virtual void mult(IDataType_ptr /*y*/) {
      throw INJECTION_EXCEPTION("DataType %d does not support function mult", key);
  }

  virtual std::string typeId() {
     throw INJECTION_EXCEPTION("DataType %d does not support function typeId", key);
  }
  virtual std::string displayName() {
     throw INJECTION_EXCEPTION("DataType %d does not support function displayName", key);
  }


  void setKey(long long key);
  long long getKey();
  virtual ~IDataType();
};

typedef IDataType* (*dataType_ptr)();

void registerDataType(std::string packageName, std::string name,
                      VnV::dataType_ptr r);

template <typename T>
void registerDataType(std::string packageName, VnV::dataType_ptr ptr) {
  std::string s = typeid(T).name();
  VnV::registerDataType(packageName, s, ptr);
}

}  // namespace VnV

#define INJECTION_DATATYPE(PNAME, name, cls)                     \
  namespace VnV {                                                \
  namespace PNAME {                                              \
  namespace DataTypes {                                          \
  VnV::IDataType* declare_##name();                              \
  void register_##name() {                                       \
    VnV::registerDataType<cls>(VNV_STR(PNAME), &declare_##name); \
  }                                                              \
  }                                                              \
  }                                                              \
  }                                                              \
  VnV::IDataType* VnV::PNAME::DataTypes::declare_##name()


#define INJECTION_TDATATYPE(PNAME,cname,cls) \
   INJECTION_DATATYPE(PNAME,cname,cls) {          \
     class datatype : public VnV::IDataType { \
        std::string typeId() override { return typeid(cls).name(); } \
        std::string displayName() override { return #cls; } \
     }; \
     return new datatype(); \
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

