
/**
  @file TransformStore.h
**/

#ifndef VVTRANSFORM_H
#define VVTRANSFORM_H
/** @file vv-testing.h
 */

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/stores/BaseStore.h"
#include "interfaces/ITransform.h"

namespace VnV {

class Transformer {
 private:
  std::string from;
  std::vector<std::pair<std::string, ITransform*>> transPath;

 public:
  Transformer(std::string from_, std::vector<std::pair<std::string, ITransform*>>& trans);
  virtual void* Transform(void* ptr);
  virtual ~Transformer();
};

/**
 * @brief The TestStore class
 */
class TransformStore : public BaseStore {
 private:
  /**
   * @brief trans_factory
   */
  std::map<std::string, trans_ptr, std::less<std::string>> trans_factory;
  std::map<std::string, std::map<std::string, std::string>> trans_map;

 public:
  /**
   * @brief TestStore
   */
  TransformStore();

  /**
   * @brief addTransform
   * @param name
   * @param p
   */
  void addTransform(std::string name, trans_ptr p, std::string from, std::string to);

  /**
   * @brief getTransform
   * @param tname
   * @return
   */
  std::shared_ptr<Transformer> getTransformer(std::string from, std::string to);

  /**
   * @brief print out test store configuration information.
   */
  void print();

  static TransformStore& instance();

  nlohmann::json schema(json& packageJson);
};

}  // namespace VnV

#endif  // VVTRANSFORM_H
