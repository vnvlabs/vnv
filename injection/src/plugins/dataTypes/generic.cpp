#include "interfaces/ICommunicator.h"
#include "base/exceptions.h"
#include <string>
#include <assert.h>
#include "c-interfaces/Logging.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace DataTypes {

// This will work with any <T> that has addition, multiplication
// and compare operators.
template <typename T> class GenericDataType : public Communication::IDataType {
  T data;

 public:
  T get() { return data; }
  void set(T d) { data = d; }

  long long maxSize() { return sizeof(T); }
  long long pack(void* buffer) { ((T*)buffer)[0] = data; }
  void unpack(void* buffer) { data = ((T*)buffer)[0]; }

  void setData(void* dat) { data = *((T*)dat); }


  void axpy(double alpha, Communication::IDataType* y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y;
    yy->set(yy->get() + alpha * data);
  }

  int compare(Communication::IDataType* y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y;
    if (yy->get() == get())
      return 0;
    else if (get() < yy->get())
      return -1;
    return 1;
  }
  // y = xy;
  void mult(Communication::IDataType* y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y;
    yy->set(yy->get() * get());
  }

  void Put(IOutputEngine* engine) override {
    engine->Put("value", get());
  }

};

#define VNV_MAX_STR_SIZE 100
class StringDataType : public Communication::IDataType {
public:
   std::string data;

 public:
  std::string get() { return data; }
  void set(std::string d) { 
   if (data.size() >= VNV_MAX_STR_SIZE) {
     throw VnV::VnVExceptionBase("String to large");
   } 
   data = d;
  }

  long long maxSize() { return sizeof(char)*(1+VNV_MAX_STR_SIZE); }
  long long pack(void* buffer) { 
    char* b = (char*) buffer;
    std::size_t length = data.copy( b , data.size());
    b[length] = '\0';
    return data.size() + 1;
     
  }
  void unpack(void* buffer) { 
    data = (char*) buffer ; 
  }

  void setData(void* dat) { 
    data = (char*) dat;
  }


  void axpy(double alpha, Communication::IDataType* y) {
    VnV_Warn(VNVPACKAGENAME, "AXPY not implemented for String");
  
  }

  int compare(Communication::IDataType* y) {
    StringDataType* yy = (StringDataType*)y;
    return yy->get().compare(get());
  }

  // y = xy;
  void mult(Communication::IDataType* y) {
    VnV_Warn(VNVPACKAGENAME, "MULT not implemented for String");
  }

  void Put(IOutputEngine* engine) override {
    engine->Put("value", get());
  }

};

}  // namespace DataTypes
}  // namespace PACKAGENAME
}  // namespace VnV

INJECTION_DATATYPE(VNVPACKAGENAME, string, std::string) {
  return new VnV::VNVPACKAGENAME::DataTypes::StringDataType();
}

INJECTION_DATATYPE(VNVPACKAGENAME,double, double) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<double>();
};

INJECTION_DATATYPE(VNVPACKAGENAME,int, int) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<int>();
};

INJECTION_DATATYPE(VNVPACKAGENAME,float, float) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<float>();
}

INJECTION_DATATYPE(VNVPACKAGENAME,long, long) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<long>();
}

INJECTION_DATATYPE(VNVPACKAGENAME, longlong, long long) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<long long>();
}

INJECTION_DATATYPE(VNVPACKAGENAME, short, short) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<short>();
}

