
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

#include "interfaces/itest.h"

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
