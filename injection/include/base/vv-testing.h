
#ifndef VV_TESTING_HEADER
#define VV_TESTING_HEADER

/** @file vv-testing.h
 */

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "VnV-Interfaces.h"
#include "vv-output.h"

namespace VnV {
/**
 * @brief The DefaultTransform class
 */
class DefaultTransform : public ITransform {
  /**
   * @brief Transform
   * @param ip
   * @param testParameterType
   * @return
   */
  void* Transform(std::pair<std::string, void*> ip,
                  std::string testParameterType) override;
};
/**
 * @brief The TestStore class
 */
class TestStore {
 private:
  /**
   * @brief testLibraries
   */
  std::vector<void*> testLibraries;

  /**
   * @brief registeredTests
   */
  std::set<std::string> registeredTests;

  /**
   * @brief test_factory
   */
  std::map<std::string, std::pair<maker_ptr*, variable_register_ptr*>,
           std::less<std::string>>
      test_factory;

  /**
   * @brief trans_factory
   */
  std::map<std::string, trans_ptr*, std::less<std::string>> trans_factory;

  /**
   * @brief TestStore
   */
  TestStore();

 public:
  /**
   * @brief addTestLibrary
   * @param libraryPath
   */
  void addTestLibrary(std::string libraryPath);

  /**
   * @brief addTest
   * @param name
   * @param m
   * @param v
   */
  void addTest(std::string name, maker_ptr m, variable_register_ptr v);

  /**
   * @brief addTransform
   * @param name
   * @param p
   */
  void addTransform(std::string name, trans_ptr p);

  /**
   * @brief getTransform
   * @param tname
   * @return
   */
  ITransform* getTransform(std::string tname);

  /**
   * @brief getTest
   * @param config
   * @return
   */
  ITest* getTest(TestConfig& config);

  /**
   * @brief getTestStore
   * @return
   */
  static TestStore& getTestStore();
};

}  // namespace VnV

#endif
