
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

#include "base/stores/BaseStore.h"
#include "json-schema.hpp"
#include "interfaces/ITest.h"
#include "base/TemplateCallback.h"
#include "base/InjectionPointConfig.h"

using nlohmann::json;

namespace VnV {

// Forward Declare
typedef std::map<std::string, std::pair<std::string, void*>> NTV;
class InjectionPoint;
class TestConfig;
class SamplerConfig;
enum class InjectionPointType;

/**
 * \class InjectionPointStore
 * @brief The InjectionPointStore class
 *
 * The InjectionPointStore manages the creation of all InjectionPoints. An new
 * InjectionPoint is created each time a INJECTION_POINT macro is found. The
 * InjectionPoints are initialized using a <name> and <TestConfig> extracted
 * from the users input file. For that reason, the InjectionPointStore keeps a
 * map of all configurations available.
 *
 * The class follows a singleton static initialization pattern. That is, a
 * static instance of the class is created on the first call to
 * getInjectionPointStore(). After that, the same instance is returned each
 * time.
 *
 * @todo We should provide an interface for hot-patching a running application.
 * The idea would be to check some user defined log file for changes prior to
 * creating an InjectionPoint. This would allow the user to turn on/off testing
 * while the program is running...
 */
class InjectionPointSpec {
 public:
  std::string package;
  std::string name;
  json specJson;
  InjectionPointSpec(std::string package, std::string name, json& spec) {
    this->package = package;
    this->name = name;
    this->specJson = spec;
  }
};


class InjectionPointStore : public BaseStore {
  std::map<std::string, std::stack<std::shared_ptr<InjectionPoint>>> active; /**< Active injection point stack*/
  std::map<std::string, InjectionPointConfig> injectionPoints;               /**< The stored configurations */
  std::map<std::string, InjectionPointSpec> registeredInjectionPoints;

  /**
   * @brief newInjectionPoint
   * @param key The name of the injection point
   * @return Shared pointer to the new Injection point
   *
   * Create a new Injection point based on the users specification.
   */

  std::shared_ptr<InjectionPoint> newInjectionPoint(std::string packageName, std::string name,
                                                    const VnV::TemplateCallback& templateCallback, NTV& in_args);

  std::shared_ptr<InjectionPoint> fetchFromQueue(std::string packageName, std::string name, InjectionPointType stage);



  void registerLoopedInjectionPoint(std::string packageName, std::string name, std::shared_ptr<InjectionPoint>& ptr);

  // JsonObject is a json object that validates againt the Injection point
  // schema.
  void registerInjectionPoint(std::string packageName, std::string id, json& jsonObject);

 public:
  /**
   * @brief InjectionPointStore
   * Private constructor. The class creates itself on first use.
   */
  InjectionPointStore();

 

  /**
   * @brief getInjectionPoint
   * @param key The name of the injection point requested.
   * @param stage The stage of the requested injection point.
   * @return The injection point requested
   *
   * Injection points are staged. This means that we need to somehow figure out
   * which injection point is being requested. Injection points can also be
   * nested, and/or called recursively. This function get the correct injection
   * point for the current stage.
   *
   * Injection Points work based on the principle of LIFO. A injection point
   * is added to the queue when a stage==0 is found. An injection point is
   * removed from the queue when stage=9999 is found. A stage==-1 indicates a
   * single stage injection point, so nothing is added to the queue. All other
   * stage values return that last injection point in the queue. A stage>0 with
   * an empty queue is considered invalid and will return a nullptr.
   *
   */
  std::shared_ptr<InjectionPoint> getNewInjectionPoint(std::string package, std::string name,
                                                       const VnV::TemplateCallback& templateCallback,
                                                       InjectionPointType type, NTV& in_args);

  std::shared_ptr<InjectionPoint> getExistingInjectionPoint(std::string package, std::string name,
                                                            InjectionPointType type);

  /**
   * @brief addInjectionPoint
   * @param name The name of the injection point
   * @param tests A vector of tests configurations describing tests to be run at
   * this injection point.
   *
   * Add an injection point to the store. In the case that an injection point
   * already exists in the store, the test configuration will be overwritten.
   */
  void addInjectionPoint(std::string package, std::string name, bool runInternal, json& templateName, std::vector<TestConfig>& tests,
                         const SamplerConfig& config);

  // Register Injection point. JsonStr must be json that validates against the
  // injection point schema OR an array of objects that individually validate
  // against that same schema.
  void registerInjectionPoint(std::string packageName, std::string name, std::string json_str);

  /**
   * @brief print out injection point infomration.
   */
  void print();

  static InjectionPointStore& instance();

  nlohmann::json schema();

};  // end InjectionPointStore

}  // namespace VnV

#endif  // INJECTIONPOINTSTORE_H
