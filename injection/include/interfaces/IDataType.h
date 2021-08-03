#ifndef VNV_IDATATYPE_HEADER
#define VNV_IDATATYPE_HEADER

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <base/exceptions.h>

namespace VnV { 

class IOutputEngine; 

class IDataType;
typedef std::shared_ptr<IDataType> IDataType_ptr;
typedef std::vector<IDataType_ptr> IDataType_vec;

enum class SupportedDataType {
    DOUBLE, LONG, STRING, JSON
};

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

  virtual void setData(void* data) { throw VnV::VnVExceptionBase("unsupported operation"); } 
   // set from a raw pointer to the data.

  // Put will be called when you need to "Write" this output to file.
  // Put is called when someone type Put("name","datatype"). This is
  // collective on the Communicator used to type Put. So, this function
  // can output things like global vectors.
  // Put should call other Put actions
  virtual void Put(VnV::IOutputEngine* engine) {
     
  }  
  

  // getLocalPutData is called when someone tries to write
  // a global vector containing objects of the this datatype
  // This is a "local" operation across the communicator set within
  // the engine.

  // This should be used to support writing vectors of a datatype. I.e., a
  // double vector. The Put is used to
  // support writing a (potentially distributed) data structure to file. This function
  // is used to support writing a distributed vector containing the data structure to
  // file.

  // The function should return a map of PutData objects that describe the different
  // parameters in the data structure.
  // THIS MUST BE CONSTANT ACROSS ALL INSTANCES OF THE DATA TYPE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // WE DO NOT SUPPORT DIFFERENT SIZES
  virtual std::map<std::string, PutData> getLocalPutData() {
      return {};
  }

  // VnV will call this function when writing a distributed vector to file that contains
  // this data structure. The parameter name will be one of the keys returned in the getLocalPutData
  // map. The function should return a pointer to the data associated with this key.
  virtual void* getPutData(std::string name) {
      throw VnV::VnVExceptionBase("Unrecognized name;");
  }

  // Needed if you want to support data type redcution and vectors.  
  virtual long long maxSize() { throw VnVExceptionBase("Unsupported Operation"); }  // what is the maximum size of the buffer
  virtual long long pack(void* buffer) { throw VnVExceptionBase("Unsupported Operation"); }  // pack the buffer
  virtual void unpack(void* buffer) { throw VnVExceptionBase("Unsupported Operation"); }     // unpack into a buffer
  
  // Needed if you want to use reduction operations. 
  virtual void axpy(double alpha, IDataType_ptr y) { throw VnVExceptionBase("Unsupported Operation"); }  // y = ax + y
  virtual int compare(IDataType_ptr y) { throw VnVExceptionBase("Unsupported Operation"); }  // -1 less, 0 == , 1 greater.
  virtual void mult(IDataType_ptr y) { throw VnVExceptionBase("Unsupported Operation"); }


  void setKey(long long key);
  long long getKey();
  virtual ~IDataType();
};

typedef IDataType* (*dataType_ptr)();

void registerDataType(std::string packageName, std::string name,
                      VnV::dataType_ptr r);

template <typename T>
void registerDataType(std::string packageName,
                      VnV::dataType_ptr ptr) {
    
    std::string s = typeid(T).name();
    VnV::registerDataType(packageName, s, ptr);

}

}

#define INJECTION_DATATYPE(PNAME, name, cls)                                   \
  namespace VnV {                                                              \
  namespace PNAME {                                                            \
  namespace DataTypes {                                                        \
  VnV::IDataType* declare_##name();                             \
  void register_##name() {                                                     \
    VnV::registerDataType<cls>(VNV_STR(PNAME), declare_##name); \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  VnV::IDataType* VnV::PNAME::DataTypes::declare_##name()

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