#ifndef VVTRANSFORM_H
#define VVTRANSFORM_H
/** @file vv-testing.h
 */

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>

#include "interfaces/ITransform.h"

namespace VnV {


/**
 * @brief The TestStore class
 */
class TransformStore {

private:
  /**
   * @brief trans_factory
   */
  std::map<std::string, trans_ptr*, std::less<std::string>> trans_factory;
  std::map<std::string, std::map<std::string, std::string>> trans_map;
  /**
   * @brief TestStore
   */
  TransformStore();

 public:

  /**
   * @brief addTransform
   * @param name
   * @param p
   */
  void addTransform(std::string name, trans_ptr p, declare_transform_ptr v);

  /**
   * @brief getTransform
   * @param tname
   * @return
   */
  void* getTransform(std::string from, std::string to, void* ptr);


  /**
   * @brief getTestStore
   * @return
   */
  static TransformStore& getTransformStore();

  /**
   * @brief print out test store configuration information.
   */
  void print();

};

}  // namespace VnV

#endif // VVTRANSFORM_H
