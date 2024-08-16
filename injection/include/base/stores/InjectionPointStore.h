
/**
  @file InjectionPointStore.h
**/

#ifndef INJECTIONPOINTSTORE_H
#define INJECTIONPOINTSTORE_H

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <deque>

#include "base/FunctionSigniture.h"
#include "base/InjectionPointConfig.h"
#include "base/stores/BaseStore.h"

using nlohmann::json;

namespace VnV
{

  // Forward Declare
  typedef std::map<std::string, std::pair<std::string, void *>> NTV;
  class InjectionPoint;
  class TestConfig;
  class SamplerConfig;

  class InjectionPointStore : public BaseStore
  {

    std::deque<std::shared_ptr<InjectionPoint>> active;          /**< Active injection point stack*/
    std::map<std::string, InjectionPointConfig> injectionPoints; /**< The stored configurations */
    std::map<std::string, InjectionPointSpec> registeredInjectionPoints;

    std::shared_ptr<InjectionPoint> newInjectionPoint(std::string packageName, std::string name,
                                                      struct VnV_Function_Sig pretty, NTV &args);

    std::shared_ptr<InjectionPoint> fetchFromQueue(std::string packageName, std::string name, InjectionPointType stage);

    void registerLoopedInjectionPoint(std::string packageName, std::string name, std::shared_ptr<InjectionPoint> &ptr);


    void registerInjectionPoint(std::string packageName, std::string id, json &jsonObject);

  public:
    /**
     * @brief InjectionPointStore
     * Private constructor. The class creates itself on first use.
     */
    InjectionPointStore();


    std::shared_ptr<InjectionPoint> getNewInjectionPoint(std::string package, std::string name,
                                                         struct VnV_Function_Sig pretty, InjectionPointType type,
                                                         NTV &args);

    std::shared_ptr<InjectionPoint> getExistingInjectionPoint(std::string package, std::string name,
                                                              InjectionPointType type);


    void addInjectionPoint(std::string package, std::string name, bool runInternal, json &templateName,
                           std::vector<TestConfig> &tests, const SamplerConfig &config);


    void registerInjectionPoint(std::string packageName, std::string name, std::string json_str);

    std::vector<std::shared_ptr<InjectionPoint>> getParents(InjectionPoint &ip, bool onqueue);


    void print();

    void runAll(std::vector<TestConfig> &tests);

    bool registered(std::string package, std::string name);
    bool registeredTest(std::string package, std::string name);

    static InjectionPointStore &instance();

    nlohmann::json schema(json &packageJson);

  }; // end InjectionPointStore

} // namespace VnV

#endif // INJECTIONPOINTSTORE_H
