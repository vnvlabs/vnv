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
  SupportedDataType dtype;
 public:

  GenericDataType(SupportedDataType type) {
    dtype = type;
  }

  T get() { return data; }
  void set(T d) { data = d; }

  long long maxSize() { return sizeof(T); }
  long long pack(void* buffer) {
      ((T*)buffer)[0] = data;
      return sizeof(T);
  }
  void unpack(void* buffer) {
      T* tbuffer = (T*) buffer;
      data = tbuffer[0];
  }

  void setData(void* dat) { data = *((T*)dat); }

  // this = this + ay
  void axpy(double alpha, Communication::IDataType_ptr y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y.get();
    data += yy->get()*alpha;
  }

  int compare(Communication::IDataType_ptr y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y.get();
    if (yy->get() == data)
      return 0;
    else if (data < yy->get())
      return -1;
    return 1;
  }
  // this = this * y;
  void mult(Communication::IDataType_ptr y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y.get();
    data = data*yy->get();
  }

  void Put(IOutputEngine* engine) override {
    engine->Put("value", get());
  }

  long long datalong;
  double datadouble;
  SupportedDataType LocalPut(std::string &name, void** data, int& writeRank, int iteration ) {

    writeRank = -1;

    if (iteration == 0 ) {
      name = "value";
      if (dtype == SupportedDataType::LONG) {
          datalong = get();
          *data = &datalong;
          return SupportedDataType::LONG;
      } else {
          datadouble = get();
          *data = &datalong;
          return SupportedDataType::DOUBLE;
      }
    }  else {
      return SupportedDataType::DONE;
    }
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


  void axpy(double alpha, Communication::IDataType_ptr y) {
    VnV_Warn(VNVPACKAGENAME, "AXPY not implemented for String");
  
  }

  int compare(Communication::IDataType_ptr y) {
    StringDataType* yy = (StringDataType*)y.get();
    return yy->get().compare(get());
  }

  // y = xy;
  void mult(Communication::IDataType_ptr y) {
    VnV_Warn(VNVPACKAGENAME, "MULT not implemented for String");
  }

  void Put(IOutputEngine* engine) override {
    engine->Put("value", get());
  }


  SupportedDataType LocalPut(std::string &name, void** data, int& writeRank, int iteration ) {
      if (iteration == 0 ) {
         name = "value";
         *data = &data;
         writeRank = -1;
         return SupportedDataType::STRING;
      }  else {
         return SupportedDataType::DONE;
      }
  }


};

}  // namespace DataTypes
}  // namespace PACKAGENAME
}  // namespace VnV

INJECTION_DATATYPE(VNVPACKAGENAME, string, std::string) {
  return new VnV::VNVPACKAGENAME::DataTypes::StringDataType();
}

INJECTION_DATATYPE(VNVPACKAGENAME,double, double) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<double>(VnV::SupportedDataType::DOUBLE);
};

INJECTION_DATATYPE(VNVPACKAGENAME,int, int) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<int>(VnV::SupportedDataType::LONG);
};

INJECTION_DATATYPE(VNVPACKAGENAME,float, float) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<float>(VnV::SupportedDataType::DOUBLE);
}

INJECTION_DATATYPE(VNVPACKAGENAME,long, long) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<long>(VnV::SupportedDataType::LONG);
}

INJECTION_DATATYPE(VNVPACKAGENAME, longlong, long long) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<long long>(VnV::SupportedDataType::LONG);
}

INJECTION_DATATYPE(VNVPACKAGENAME, short, short) {
  return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<short>(VnV::SupportedDataType::LONG);
}

