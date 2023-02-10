
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

// A demo class we are trying to use in the toolkit.
class CustomDataType {
 private:
  double x;
  double y;
  double z;

 public:
  CustomDataType(double x, double y, double z) {
    setX(x);
    setY(y);
    setZ(z);
  }

  void setX(double x) { this->x = x; }
  void setY(double y) { this->y = y; }
  void setZ(double z) { this->z = z; }
  double& getX() { return x; }
  double& getY() { return y; }
  double& getZ() { return z; }

  void axpy(double alpha, CustomDataType* y) {
    setX(getX() + alpha * y->getX());
    setY(getY() + alpha * y->getY());
    setY(getZ() + alpha * y->getZ());
  }

  int compare(CustomDataType* y) { return (getX() + getY() + getZ()) - (y->getX() + y->getY() + y->getZ()); }

  void mult(CustomDataType* y) {
    setX(getX() * y->getX());
    setY(getY() * y->getY());
    setZ(getZ() * y->getZ());
  }
};

// The wrapper for that data type.
// All overrides are optional -- the more you add , the more
// functionality imparted on the data structure.

class ExampleDataType : public VnV::IDataType {
  bool own = false;
  CustomDataType* t = NULL;

 public:
  ~ExampleDataType() {
    if (own) {
      delete t;
    }
  }

  ExampleDataType() {}

  // Max size for the  void* buffer needed to serialize the data structure
  long long maxSize() override { return 3 * sizeof(double); }

  // serialize the data type into a void* buffer (size is maxSize )
  long long pack(void* buffer) override {
    double* b = (double*)buffer;
    b[0] = t->getX();
    b[1] = t->getY();
    b[2] = t->getZ();
    return maxSize();
  }

  // Unpack from the data structure
  void unpack(void* buffer) override {
    double* b = (double*)buffer;
    own = true;
    t = new CustomDataType(b[0], b[1], b[2]);
  }

  // this is a void* pointer to the data structure being wraped.
  void setData(void* dat) override {
    own = false;
    t = (CustomDataType*)dat;
  }

  // this = this + ay
  void axpy(double alpha, VnV::IDataType_ptr y) override { t->axpy(alpha, ((ExampleDataType*)y.get())->t); }

  // Compare to objects of the same type
  int compare(VnV::IDataType_ptr y) override { return t->compare(((ExampleDataType*)y.get())->t); }

  // this = this * y;
  void mult(VnV::IDataType_ptr y) override { t->mult(((ExampleDataType*)y.get())->t); }

  // Write the data structure to the engine.
  void Put(VnV::IOutputEngine* engine) override {
    engine->Put("x", t->getX());
    engine->Put("y", t->getY());
    engine->Put("z", t->getZ());
  }

  // Local data is used when trying to write the data structure as part
  // of a vector. This must be local to the comm. Here we describe the
  // data that will be written.
  std::map<std::string, VnV::PutData> getLocalPutData() override {
    VnV::PutData p;
    p.datatype = VnV::SupportedDataType::DOUBLE;
    p.shape = {};

    std::map<std::string, VnV::PutData> res;
    p.name = "x";
    res["x"] = p;
    p.name = "y";
    res["y"] = p;
    p.name = "z";
    res["z"] = p;
    return res;
  }

  // Return a pointer to the data associated with the name "name" as
  // outlined in the getLocalPutData function.
  void* getPutData(std::string name) override {
    if (name.compare("x") == 0) {
      return &(t->getX());
    } else if (name.compare("y") == 0) {
      return &(t->getY());
    } else if (name.compare("z") == 0) {
      return &(t->getZ());
    }
    HTHROW INJECTION_EXCEPTION("Invalid Put Data Request %s. Only x,y,z are supported", name.c_str());
  }
};

INJECTION_DATATYPE(VnVPlugin, customDataType, ExampleDataType) { return new ExampleDataType(); };