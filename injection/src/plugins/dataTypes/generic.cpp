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
template <typename T> class GenericDataType : public IDataType {
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
  void axpy(double alpha, IDataType_ptr y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y.get();
    data += yy->get()*alpha;
  }

  int compare(IDataType_ptr y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y.get();
    if (yy->get() == data)
      return 0;
    else if (data < yy->get())
      return -1;
    return 1;
  }
  // this = this * y;
  void mult(IDataType_ptr y) {
    GenericDataType<T>* yy = (GenericDataType<T>*)y.get();
    data = data*yy->get();
  }

  void Put(IOutputEngine* engine) override {
    engine->Put("value", get());
  }

  long long datalong;
  double datadouble;
  std::map<std::string,PutData> getLocalPutData() override {
    PutData p;
    p.datatype = dtype;
    p.shape = {};
    p.name = "value";
    return {std::make_pair("value",p)};
  }


  void* getPutData(std::string name) override {
      return &data;
  }

};

template<unsigned int N, typename T>
class StringDataType : public IDataType {
public:
   T data;
   std::function<std::string(const T&)> toString;
   std::function<T(char *)> fromCharStar;
   std::function<int(const T&,const T&)> compareT;
   SupportedDataType dtype;

   StringDataType(
                   SupportedDataType dtype,
                   std::function<std::string(const T&)> toString,
                   std::function<T(char *)> fromCharStar,
                   std::function<int(const T&,const T&)> compareT
                  ) {

     this->compareT = compareT;
     this->fromCharStar = fromCharStar;
     this->toString = toString;
     this->dtype = dtype;
   }

public:
  std::string get() { return data; }

  void set(T d) {
   if (toString(data).size() >= N) {
     throw VnV::VnVExceptionBase("String to large");
   } 
   data = d;
  }
  void setData(void* dat) {
    data =  *((T*) dat);
  }

  long long maxSize() { return sizeof(char)*(1+N); }

  long long pack(void* buffer) {
    char* b = (char*) buffer;
    std::string str = toString(data);
    std::size_t length = str.copy( b , str.size());
    b[length] = '\0';
    return str.size() + 1;
  }

  void unpack(void* buffer) {
    data = fromCharStar((char*) buffer) ;
  }

  void axpy(double alpha, IDataType_ptr y) {
    throw VnV::VnVExceptionBase("axpy not supported for string data types");
  }

  int compare(IDataType_ptr y) {
    StringDataType* yy = (StringDataType*)y.get();
    return compareT(get(),yy->get());
  }

  void mult(IDataType_ptr y) {
    throw VnV::VnVExceptionBase("multiplication not supported for string data types");
  }

  void Put(IOutputEngine* engine) override {
    engine->Put("value", get());
  }

  std::map<std::string,PutData> getLocalPutData() override {
    PutData p;
    p.datatype = dtype;
    p.shape = {};
    p.name = "value";
    return {std::make_pair("value",p)};
  }
  void* getPutData(std::string name) override {
    return &data;
  }
};

}  // namespace DataTypes
}  // namespace PACKAGENAME
}  // namespace VnV

INJECTION_DATATYPE(VNVPACKAGENAME, string, std::string) {
  return new VnV::VNVPACKAGENAME::DataTypes::StringDataType<1024,std::string>(
      SupportedDataType::STRING,
      [](const std::string &a){return a; },
      [](char * a){ return std::string(a);},
      [](const std::string &o1, const std::string &o2){ return o1.compare(o2);}
  );
}

INJECTION_DATATYPE(VNVPACKAGENAME, json, nlohmann::json) {
  return new VnV::VNVPACKAGENAME::DataTypes::StringDataType<1024,nlohmann::json>(
      SupportedDataType::STRING,
      [](const nlohmann::json &a){return a.dump(); },
      [](char * a){ return json::parse(a);},
      [](const nlohmann::json &o1, const nlohmann::json &o2){ return o1.dump().compare(o2.dump());}
  );
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

