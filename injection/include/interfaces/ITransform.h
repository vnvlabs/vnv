#ifndef ITRANSFORM_H
#define ITRANSFORM_H

#include <string>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {

/**
 * @brief The ITransform class
 */
class ITransform {
 public:
  friend class TransformStore;
  friend class Transformer;
  /**
   * @brief ITransform
   */
  ITransform();

  virtual ~ITransform();

 private:
  /**
   * @brief Transform
   * @param ip
   * @param tp
   * @return
   */
  virtual void* Transform(void* ptr) = 0;
};

typedef ITransform* (*trans_ptr)();
void registerTransform(std::string name, VnV::trans_ptr t, std::string from, std::string to);

template <typename To, typename From, typename Runner> class Transform_T : public ITransform {
 public:
  std::shared_ptr<Runner> runner;
  Transform_T() : ITransform() { runner.reset(new Runner()); }

  virtual To* Transform(From* ptr) = 0;

  void* Transform(void* ptr) override { return (void*)Transform((From*)ptr); }
};

}  // namespace VnV

#define INJECTION_TRANSFORM_INTERNAL(PNAME, NAME, Runner, From, To)                                                \
  namespace VnV {                                                                                                  \
  namespace PNAME {                                                                                                \
  namespace Transforms {                                                                                           \
  class NAME : public VnV::Transform_T<VnV_Arg_Type(To), VnV_Arg_Type(From), VnV_Arg_Type(Runner)> {               \
   public:                                                                                                         \
    NAME() : Transform_T<VnV_Arg_Type(To), VnV_Arg_Type(From), VnV_Arg_Type(Runner)>() {}                          \
    VnV_Arg_Type(To) * Transform(VnV_Arg_Type(From) * ptr);                                                        \
  };                                                                                                               \
  ITransform* declare_##NAME() { return new NAME(); }                                                              \
  void register_##NAME() {                                                                                         \
    registerTransform(#NAME, &declare_##NAME, VnV::StringUtils::get_type(#From), VnV::StringUtils::get_type(#To)); \
  }                                                                                                                \
  }                                                                                                                \
  }                                                                                                                \
  }                                                                                                                \
  VnV_Arg_Type(To) * VnV::PNAME::Transforms::NAME::Transform(VnV_Arg_Type(From) * ptr)

// Macro indirection to help clang tool support cases where these
// are macros -- TODO.
#define INJECTION_TRANSFORM_R(PNAME, NAME, Runner, From, To) INJECTION_TRANSFORM_INTERNAL(PNAME, NAME, Runner, From, To)

#define INJECTION_TRANSFORM(PNAME, name, from, to) INJECTION_TRANSFORM_R(PNAME, name, int, from, to)

#define INJECTION_ALIAS(PNAME, NAME, From, To)                                                                     \
  namespace VnV {                                                                                                  \
  namespace PNAME {                                                                                                \
  namespace Transforms {                                                                                           \
  class NAME : public ITransform {                                                                                 \
   public:                                                                                                         \
    NAME() : ITransform() {}                                                                                       \
    void* Transform(void* ptr) override { return ptr; }                                                            \
  };                                                                                                               \
  ITransform* declare_##NAME() { return new NAME(); }                                                              \
  void register_##NAME() {                                                                                         \
    registerTransform(#NAME, &declare_##NAME, VnV::StringUtils::get_type(#From), VnV::StringUtils::get_type(#To)); \
  }                                                                                                                \
  }                                                                                                                \
  }                                                                                                                \
  }

#define DECLARETRANSFORM(PNAME, name) \
  namespace VnV {                     \
  namespace PNAME {                   \
  namespace Transforms {              \
  void register_##name();             \
  }                                   \
  }                                   \
  }

#define REGISTERTRANSFORM(PNAME, name) VnV::PNAME::Transforms::register_##name();

#endif  // ITRANSFORM_H
