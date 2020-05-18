#ifndef ISERIALIZER_H
#define ISERIALIZER_H

#include <map>
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
  virtual std::string Serialize(std::string inputType, void* ptr ) = 0;
};

typedef ISerializer* serializer_ptr();

void registerSerializer(std::string package, std::string name, VnV::serializer_ptr t, std::string type);

// INJECTION_TRANSFORM(name, a->b, b->c, a->c, )
}
namespace{
template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
}

#define INJECTION_SERIALIZER_R(NAME, Runner, TYPE) \
namespace VnV{ \
namespace PACKAGENAME {\
namespace Serializer { \
class NAME : public ISerializer { \
public:\
    std::string inputType;\
    std::shared_ptr<Runner> runner; \
    NAME() : ISerializer() { \
          inputType == #TYPE;\
          runner.reset(new Runner()); \
    }\
    std::string Serialize(argument_type<void(TYPE)>::type* ptr);\
    std::string Serialize(std::string inputType, void*ptr) {\
        if (inputType == this->inputType) {\
           return Serialize((argument_type<void(TYPE)>::type*) ptr);\
        }\
    }\
};\
ISerializer* declare_##NAME() { return new NAME(); } \
void register_##NAME() { \
    registerSerializer(PACKAGENAME_S, #NAME, declare_##NAME, #TYPE); \
} \
} \
} \
} \
std::string VnV::PACKAGENAME::Serializer::NAME::Serialize(argument_type<void(TYPE)>::type* ptr)

#define INJECTION_SERIALIZER(name,TYPE) INJECTION_SERIALIZER_R(name,int,TYPE)


#define DECLARESERIALIZER(name) \
  namespace VnV { namespace PACKAGENAME { namespace Serializer { void register_##name(); } } }

#define REGISTERSERIALIZER(name) \
  VnV::PACKAGENAME::Serializer::register_##name();


#endif // ISERIALIZER_H
