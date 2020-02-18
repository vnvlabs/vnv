#ifndef ITRANSFORM_H
#define ITRANSFORM_H

#include <string>
#include "json-schema.hpp"
using nlohmann::json;

namespace VnV {


/**
 * @brief The ITransform class
 */
class ITransform {
 public:
  friend class TransformStore;
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
  virtual void* Transform(std::string outputType, std::string inputType, void* ptr ) = 0;

};

typedef ITransform* trans_ptr();
typedef json declare_transform_ptr();
void registerTransform(std::string name, VnV::trans_ptr t, VnV::declare_transform_ptr v);

}
#endif // ITRANSFORM_H
