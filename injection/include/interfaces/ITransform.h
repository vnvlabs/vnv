#ifndef ITRANSFORM_H
#define ITRANSFORM_H

#include <string>
#include "json-schema.hpp"
#include "base/Utilities.h"
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
  virtual void* Transform(std::string from,  std::string to, void* ptr, std::string &rtti );


};

typedef ITransform* trans_ptr();
void registerTransform(std::string name, VnV::trans_ptr t, std::string from, std::string to);

namespace{
template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };
}

}



#define INJECTION_TRANSFORM_R(NAME, Runner, FROM, TO ) \
namespace VnV{ \
namespace PACKAGENAME {\
namespace Transforms {\
class NAME : public ITransform { \
public:\
    std::string from; \
    std::string to; \
    std::shared_ptr<Runner> runner; \
    NAME() : ITransform() { \
        from = VnV::StringUtils::trim_copy(#FROM);\
        to = VnV::StringUtils::trim_copy(#TO);\
        if ( from[0] == '(' && from[from.size()-1] == ')' ) {\
            from = from.substr(1,from.size()-1);}\
        if ( to[0] == '(' && to[to.size()-1] == ')' ) {\
            to = to.substr(1,to.size()-1);}\
        runner.reset(new Runner()); \
    }\
    \
    argument_type<void(TO)>::type* Transform(argument_type<void(FROM)>::type *ptr);\
    void* Transform(std::string from_, std::string to_, void* ptr, std::string &rtti) {\
       if (from == from_ && to == to_ ) {\
          rtti = typeid(argument_type<void(TO)>::type).name();\
          return (void*) Transform( (argument_type<void(FROM)>::type *) ptr );\
       }\
    }\
}; \
ITransform* declare_##NAME() { return new NAME(); } \
void register_##NAME() { \
    VnV::registerTransform(#NAME, declare_##NAME, VnV::StringUtils::trim_copy(#FROM), VnV::StringUtils::trim_copy(#TO)); \
} \
} \
} \
} \
VnV::argument_type<void(TO)>::type* VnV::PACKAGENAME::Transforms::NAME::Transform(VnV::argument_type<void(FROM)>::type *ptr)

#define INJECTION_TRANSFORM(name,from, to) INJECTION_TRANSFORM_R(name,int,from,to)

#define DECLARETRANSFORM(name) \
  namespace VnV { namespace PACKAGENAME { namespace Transforms { void register_##name(); } } }

#define REGISTERTRANSFORM(name) \
  VnV::PACKAGENAME::Transforms::register_##name();

#endif // ITRANSFORM_H
