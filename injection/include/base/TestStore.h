
/**
  @file TestStore.h
**/
#ifndef VV_TESTING_HEADER
#define VV_TESTING_HEADER

/** @file vv-testing.h
 */

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "interfaces/ITest.h"

namespace VnV {

/**
 * @brief The TestStore class
 */
class TestStore {
 private:

  static json& defaultSchemaJson();

  /**
   * @brief testLibraries
   */
  std::vector<void*> testLibraries;
  std::vector<std::string> testLibraryPaths;
  /**

   * @brief registeredTests
   */
  std::map<std::string, json> registeredTests;

  /**
   * @brief test_factory
   */
  std::map<std::string,
           std::pair<maker_ptr*, std::map<std::string, std::string>>,
           std::less<std::string>>

      test_factory;

  std::map<std::string, json > schema_factory;

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
  void addTest(std::string package, std::string name, maker_ptr m,
               std::map<std::string, std::string> v);

  void addSchema(std::string package, std::string name, json& schema);
  bool verifySchema(std::string package, std::string name, json& opts);

  /**
   * @brief getTest
   * @param config
   * @return
   */
  std::shared_ptr<ITest> getTest(TestConfig& config);

  std::vector<TestConfig> validateTests(std::vector<json>& configs);

  TestConfig validateTest(json& config);


  json& getSchema(std::string package, std::string name);

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
