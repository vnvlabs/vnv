
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

#include "base/stores/BaseStore.h"
#include "interfaces/ITest.h"

namespace VnV
{

  class TestInfo
  {
  public:
    maker_ptr maker;
    json schema;

    TestInfo(maker_ptr ptr, nlohmann::json s)
    {
      maker = ptr;
      schema = s;
    }
  };

  /**
   * @brief The TestStore class
   */
  class TestStore : public BaseStore
  {
  public:
    TestStore() {}

    std::map<std::string, json> registeredTests;

    std::map<std::string, TestInfo, std::less<std::string>> test_factory;

    void addTest(std::string package, std::string name, std::string s, maker_ptr m);
    // TODO -- THIS SHOULD RETURN THE VALIDATION ERROR SOMEHOW>
    bool verifySchema(std::string package, std::string name, json &opts);

    std::shared_ptr<ITest> getTest(TestConfig &config);

    nlohmann::json &defaultSchemaJson();

    std::vector<TestConfig> validateTests(std::vector<json> &configs);

    TestConfig validateTest(json &testJson);

    json &getSchema(std::string package, std::string name);

    nlohmann::json schema(json &packageJson);

    void print();

    static TestStore &instance();
  };

} // namespace VnV

#endif
