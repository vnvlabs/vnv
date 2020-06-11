#include "interfaces/ICommunicator.h"

namespace VnV {
namespace PACKAGENAME {
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
};

}  // namespace DataTypes
}  // namespace PACKAGENAME
}  // namespace VnV

INJECTION_DATATYPE(double) {
  return new VnV::PACKAGENAME::DataTypes::GenericDataType<double>();
}

INJECTION_DATATYPE(int) {
  return new VnV::PACKAGENAME::DataTypes::GenericDataType<int>();
}

INJECTION_DATATYPE(float) {
  return new VnV::PACKAGENAME::DataTypes::GenericDataType<float>();
}

INJECTION_DATATYPE(long) {
  return new VnV::PACKAGENAME::DataTypes::GenericDataType<long>();
}

INJECTION_DATATYPE(longlong) {
  return new VnV::PACKAGENAME::DataTypes::GenericDataType<long long>();
}

INJECTION_DATATYPE(short) {
  return new VnV::PACKAGENAME::DataTypes::GenericDataType<short>();
}
