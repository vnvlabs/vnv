#ifndef VNV_IREDUCTION_HEADER
#define VNV_IREDUCTION_HEADER

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace VnV {

class IDataType;
typedef std::shared_ptr<IDataType> IDataType_ptr;

class IReduction;
typedef std::shared_ptr<IReduction> IReduction_ptr;
typedef std::vector<IReduction_ptr> IReduction_vec;

class IReduction {
 private:
  long long key;

 public:
  virtual ~IReduction() {}

  long long getKey();
  void setKey(long long key);

  virtual bool communitive() = 0;  // Is this communative.
  virtual IDataType_ptr reduce(IDataType_ptr& in, IDataType_ptr& out) = 0;
};

typedef IReduction* (*reduction_ptr)();

void registerReduction(std::string packageName, std::string name, VnV::reduction_ptr r);

}  // namespace VnV

#define INJECTION_REDUCER(PNAME, name, COMM)                                                 \
  namespace VnV {                                                                            \
  namespace PNAME {                                                                          \
  namespace Reducers {                                                                       \
  class name : public IReduction {                                                           \
    virtual bool communitive() { return COMM; }                                              \
    virtual IDataType_ptr reduce(VnV::IDataType_ptr& a, VnV::IDataType_ptr& b);              \
  };                                                                                         \
                                                                                             \
  VnV::IReduction* declare_##name() { return new name(); }                                   \
  void register_##name() { VnV::registerReduction(VNV_STR(PNAME), #name, &declare_##name); } \
  }                                                                                          \
  }                                                                                          \
  }                                                                                          \
  VnV::IDataType_ptr VnV::PNAME::Reducers::name::reduce(VnV::IDataType_ptr& in, VnV::IDataType_ptr& out)

#define DECLAREREDUCER(PNAME, name) \
  namespace VnV {                   \
  namespace PNAME {                 \
  namespace Reducers {              \
  void register_##name();           \
  }                                 \
  }                                 \
  }

#define REGISTERREDUCER(PNAME, name) VnV::PNAME::Reducers::register_##name();

#endif