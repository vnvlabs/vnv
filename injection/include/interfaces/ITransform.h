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
  virtual void* Transform(std::string from, std::string to, void* ptr,
                          std::string& rtti);
};

typedef ITransform* trans_ptr();
void registerTransform(std::string name, VnV::trans_ptr t, std::string from,
                       std::string to);

template <typename To, typename From, typename Runner>
class Transform_T : public ITransform {
 public:
  std::string from;
  std::string to;
  std::shared_ptr<Runner> runner;
  Transform_T(std::string to, std::string from) : ITransform() {
    from = VnV::StringUtils::get_type(from);
    to = VnV::StringUtils::get_type(to);
    runner.reset(new Runner());
  }

  virtual To* Transform(From* ptr) = 0;
  void* Transform(std::string from_, std::string to_, void* ptr,
                  std::string& rtti) {
    if (from == from_ && to == to_) {
      rtti = typeid(To).name();
      return (void*)Transform((From*)ptr);
    }
    throw VnVExceptionBase("Bad Transform");
  }
};

}  // namespace VnV

#define INJECTION_TRANSFORM_R(NAME, Runner, To, From)                        \
  namespace VnV {                                                            \
  namespace PACKAGENAME {                                                    \
  namespace Transforms {                                                     \
  class NAME : public VnV::Transform_T<VnV_Arg_Type(To), VnV_Arg_Type(From), \
                                       VnV_Arg_Type(Runner)> {               \
   public:                                                                   \
    NAME()                                                                   \
        : Transform_T<VnV_Arg_Type(To), VnV_Arg_Type(From),                  \
                      VnV_Arg_Type(Runner)>(#To, #From) {}                   \
    VnV_Arg_Type(To) * Transform(VnV_Arg_Type(From) * ptr);                  \
  };                                                                         \
  ITransform* declare_##NAME() { return new NAME(); }                        \
  void register_##NAME() {                                                   \
    registerTransform(#NAME, declare_##NAME,                                 \
                      VnV::StringUtils::get_type(#From),                     \
                      VnV::StringUtils::get_type(#To));                      \
  }                                                                          \
  }                                                                          \
  }                                                                          \
  }                                                                          \
  VnV_Arg_Type(To) *                                                         \
      VnV::PACKAGENAME::Transforms::NAME::Transform(VnV_Arg_Type(From) * ptr)

#define INJECTION_TRANSFORM(name, from, to) \
  INJECTION_TRANSFORM_R(name, int, from, to)

#define DECLARETRANSFORM(name) \
  namespace VnV {              \
  namespace PACKAGENAME {      \
  namespace Transforms {       \
  void register_##name();      \
  }                            \
  }                            \
  }

#define REGISTERTRANSFORM(name) VnV::PACKAGENAME::Transforms::register_##name();

#endif  // ITRANSFORM_H
