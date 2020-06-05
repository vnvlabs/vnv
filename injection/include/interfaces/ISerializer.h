#ifndef ISERIALIZER_H
#define ISERIALIZER_H

#include <map>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {
/**
 * @brief The ITransform class
 */
class ISerializer {
 public:
  friend class SerializerStore;
  /**
   * @brief ITransform
   */
  ISerializer();

  virtual ~ISerializer();

 public:
  virtual std::string Serialize(std::string inputType, void* ptr) = 0;
};

typedef ISerializer* serializer_ptr();

void registerSerializer(std::string package, std::string name,
                        VnV::serializer_ptr t, std::string type);

// INJECTION_TRANSFORM(name, a->b, b->c, a->c, )

template <typename T, typename V> class Serializer_T : public ISerializer {
 public:
  std::string inputType;
  std::shared_ptr<V> runner;
  Serializer_T(const char* typeName) {
    std::string type = VnV::StringUtils::get_type(typeName);
    if (type[0] == '(' && type[type.size() - 1] == ')') {
      inputType = type.substr(1, type.size() - 1);
    } else {
      inputType = type;
    }
    runner.reset(new V());
  }
  virtual std::string Serialize(T* ptr) = 0;
  std::string Serialize(std::string type, void* ptr) {
    if (type == inputType) {
      return Serialize((T*)ptr);
    }
    throw VnVExceptionBase("Bad Type for serializer");
  }
};

}  // namespace VnV

#define INJECTION_SERIALIZER_R(NAME, Runner, TYPE)                             \
  namespace VnV {                                                              \
  namespace PACKAGENAME {                                                      \
  namespace Serializer {                                                       \
  class NAME : public Serializer_T<VnV_Arg_Type(TYPE), VnV_Arg_Type(Runner)> { \
   public:                                                                     \
    NAME() : Serializer_T<VnV_Arg_Type(TYPE), VnV_Arg_Type(Runner)>(#TYPE) {}  \
    std::string Serialize(VnV_Arg_Type(TYPE) * ptr) override;                  \
  };                                                                           \
  ISerializer* declare_##NAME() { return new NAME(); }                         \
  void register_##NAME() {                                                     \
    registerSerializer(PACKAGENAME_S, #NAME, declare_##NAME, #TYPE);           \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  }                                                                            \
  std::string VnV::PACKAGENAME::Serializer::NAME::Serialize(                   \
      VnV_Arg_Type(TYPE) * ptr)

#define INJECTION_SERIALIZER(name, TYPE) INJECTION_SERIALIZER_R(name, int, TYPE)

#define DECLARESERIALIZER(name) \
  namespace VnV {               \
  namespace PACKAGENAME {       \
  namespace Serializer {        \
  void register_##name();       \
  }                             \
  }                             \
  }

#define REGISTERSERIALIZER(name) \
  VnV::PACKAGENAME::Serializer::register_##name();

#endif  // ISERIALIZER_H
