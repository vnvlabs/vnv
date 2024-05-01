#include <assert.h>

#include <string>

#include "shared/exceptions.h"
#include "common-interfaces/Logging.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace DataTypes {

// This will work with any <T> that has addition, multiplication
// and compare operators.
template <typename T, const char* dname> class GenericDataType : public IDataType {
  T data;
  SupportedDataType dtype;

 public:
  GenericDataType(SupportedDataType type) { dtype = type; }

  T get() { return data; }
  void set(T d) { data = d; }

  long long maxSize() override { return sizeof(T); }
  long long pack(void* buffer) override {
    ((T*)buffer)[0] = data;
    return sizeof(T);
  }
  void unpack(void* buffer) override {
    T* tbuffer = (T*)buffer;
    data = tbuffer[0];
  }

  void setData(void* dat) override { data = *((T*)dat); }

  // this = this + ay
  void axpy(double alpha, IDataType_ptr y)override {
    GenericDataType<T, dname>* yy = (GenericDataType<T, dname>*)y.get();
    data += yy->get() * alpha;
  }

  int compare(IDataType_ptr y) override {
    GenericDataType<T, dname>* yy = (GenericDataType<T, dname>*)y.get();
    if (yy->get() == data)
      return 0;
    else if (data < yy->get())
      return -1;
    return 1;
  }
  // this = this * y;
  void mult(IDataType_ptr y) override {
    GenericDataType<T, dname>* yy = (GenericDataType<T, dname>*)y.get();
    data = data * yy->get();
  }

  std::string typeId() override { return typeid(T).name(); }

  std::string displayName() override { return dname; }

  void Put(IOutputEngine* engine) override { engine->Put("value", get()); }

  long long datalong;
  double datadouble;
  std::map<std::string, PutData> getLocalPutData() override {
    PutData p;
    p.datatype = dtype;
    p.shape = {};
    p.name = "value";
    return {std::make_pair("value", p)};
  }

  void* getPutData(std::string name) override { return &data; }
};

template <unsigned int N, typename T, const char* dname> class StringDataType : public IDataType {
 public:
  T data;
  std::function<std::string(const T&)> toString;
  std::function<T(char*)> fromCharStar;
  std::function<int(const T&, const T&)> compareT;
  SupportedDataType dtype;

  StringDataType(SupportedDataType dtype, std::function<std::string(const T&)> toString,
                 std::function<T(char*)> fromCharStar, std::function<int(const T&, const T&)> compareT) {
    this->compareT = compareT;
    this->fromCharStar = fromCharStar;
    this->toString = toString;
    this->dtype = dtype;
  }

 public:
  std::string get() { return data; }

  void set(T d) {
    if (toString(data).size() >= N) {
      throw INJECTION_EXCEPTION("String to large %d >= %d ", toString(data).size(), N);
    }
    data = d;
  }
  void setData(void* dat) override { data = *((T*)dat); }

  long long maxSize()override  { return sizeof(char) * (1 + N); }

  long long pack(void* buffer) override {
    char* b = (char*)buffer;
    std::string str = toString(data);
    std::size_t length = str.copy(b, str.size());
    b[length] = '\0';
    return str.size() + 1;
  }

  void unpack(void* buffer) override { data = fromCharStar((char*)buffer); }

  void axpy(double alpha, IDataType_ptr y) override { throw INJECTION_EXCEPTION_("axpy not supported for string data types"); }

  int compare(IDataType_ptr y) override {
    StringDataType* yy = (StringDataType*)y.get();
    return compareT(get(), yy->get());
  }

  void mult(IDataType_ptr y)override  { throw INJECTION_EXCEPTION_("multiplication not supported for string data types"); }

  std::string typeId() override { return typeid(T).name(); }

  std::string displayName() override { return dname; }

  void Put(IOutputEngine* engine) override { engine->Put("value", get()); }

  std::map<std::string, PutData> getLocalPutData() override {
    PutData p;
    p.datatype = dtype;
    p.shape = {};
    p.name = "value";
    return {std::make_pair("value", p)};
  }
  void* getPutData(std::string name) override { return &data; }
};

}  // namespace DataTypes
}  // namespace VNVPACKAGENAME
}  // namespace VnV

static const char stringname[] = "std::string";
INJECTION_DATATYPE(VNVPACKAGENAME, string, std::string) {
  return new VnV::VNVPACKAGENAME::DataTypes::StringDataType<1024, std::string, stringname>(
      SupportedDataType::STRING, [](const std::string& a) { return a; }, [](char* a) { return std::string(a); },
      [](const std::string& o1, const std::string& o2) { return o1.compare(o2); });
}

static const char jsonname[] = "nlohmann::json";
INJECTION_DATATYPE(VNVPACKAGENAME, json, nlohmann::json) {
  return new VnV::VNVPACKAGENAME::DataTypes::StringDataType<1024, nlohmann::json, jsonname>(
      SupportedDataType::STRING, [](const nlohmann::json& a) { return a.dump(); },
      [](char* a) { return json::parse(a); },
      [](const nlohmann::json& o1, const nlohmann::json& o2) { return o1.dump().compare(o2.dump()); });
}

#define gtypes                 \
  X(double, double, DOUBLE)    \
  X(int, int, LONG)            \
  X(float, float, LONG)        \
  X(long, long, LONG)          \
  X(longlong, long long, LONG) \
  X(short, short, LONG)        \
  X(size_t, std::size_t, LONG)

#define X(key, clas, Dtype)                                                                                     \
  static const char key##name[] = #clas;                                                                        \
  INJECTION_DATATYPE(VNVPACKAGENAME, key, clas) {                                                               \
    return new VnV::VNVPACKAGENAME::DataTypes::GenericDataType<clas, key##name>(VnV::SupportedDataType::Dtype); \
  };

gtypes

#undef x
#undef gtypes
