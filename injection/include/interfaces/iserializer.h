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
private:

  virtual void* Serialize(std::string inputType, void* ptr ) = 0;
  virtual void* deSerialize(std::string outputType, void*ptr) = 0;

};

typedef ISerializer* serializer_ptr();
typedef json declare_serializer_ptr();

void registerSerializer(std::string name, VnV::serializer_ptr t, VnV::declare_serializer_ptr v);



}
#endif // ISERIALIZER_H
