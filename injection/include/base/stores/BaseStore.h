#ifndef BASEBASESTORE_HEADER
#define BASEBASESTORE_HEADER

namespace VnV {

class BaseStore {
 public:
  BaseStore() {}
  virtual ~BaseStore() {}
};

}  // namespace VnV

#define BaseStoreInstance(CL) \
  CL& CL::instance() { return RunTime::instance().store<CL>(); }

#endif