
#ifndef VV_TESTING_HEADER
#define VV_TESTING_HEADER

/** @file vv-testing.h
 */

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <list>

#include "VnV-Interfaces.h"
#include "vv-output.h"

namespace VnV {


/**
 * @brief The TestStore class
 */
class TestStore {
 private:
  /**
   * @brief testLibraries
   */
  std::vector<void*> testLibraries;
  std::vector<std::string> testLibraryPaths;
  /**

   * @brief registeredTests
   */
  std::map<std::string, std::pair<json,json>> registeredTests;

  /**
   * @brief test_factory
   */
  std::map<std::string, std::pair<maker_ptr*, declare_test_ptr*>, std::less<std::string>>
      test_factory;

  /**
   * @brief trans_factory
   */
  std::map<std::string, trans_ptr*, std::less<std::string>> trans_factory;
  std::map<std::string, std::map<std::string, std::string>> trans_map;
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
  void addTest(std::string name, maker_ptr m, declare_test_ptr v);

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
   * @brief getTest
   * @param config
   * @return
   */
  ITest* getTest(TestConfig& config);

  std::vector<TestConfig> validateTests(std::vector<json> &configs);


  TestConfig validateTest(json &config);

  /**
   * @brief getTestStore
   * @return
   */
  static TestStore& getTestStore();

  /**
   * @brief print out test store configuration information.
   */
  void print();

};

}  // namespace VnV

#endif
