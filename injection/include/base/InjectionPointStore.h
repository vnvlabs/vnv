
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
#include "interfaces/IOutputEngine.h"
#include "base/InjectionPoint.h"

namespace VnV {
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
class InjectionPointStore {
 private:
  std::map<std::string, std::stack<std::shared_ptr<InjectionPoint>>>
      active; /**< Active injection point stack*/

  std::map<std::string, std::vector<TestConfig>>
      injectionPoints; /**< The stored configurations */

  std::map<std::string, json> registeredInjectionPoints;

  /**
   * @brief InjectionPointStore
   * Private constructor. The class creates itself on first use.
   */
  InjectionPointStore();

  /**
   * @brief newInjectionPoint
   * @param key The name of the injection point
   * @return Shared pointer to the new Injection point
   *
   * Create a new Injection point based on the users specification.
   */
  std::shared_ptr<InjectionPoint> newInjectionPoint(std::string key, va_list args);

  // JsonObject is a json object that validates againt the Injection point schema.
  void registerInjectionPoint(json &jsonObject);

 public:
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
  std::shared_ptr<InjectionPoint> getInjectionPoint(std::string key, InjectionPointType type, va_list args);

  /**
   * @brief addInjectionPoint
   * @param name The name of the injection point
   * @param tests A vector of tests configurations describing tests to be run at
   * this injection point.
   *
   * Add an injection point to the store. In the case that an injection point
   * already exists in the store, the test configuration will be overwritten.
   */
  void addInjectionPoint(std::string name, std::vector<TestConfig>& tests);

  /**
   * @brief addInjectionPoints
   * @param injectionPoints A map of injection point configurations to be added
   * to the store
   *
   * Add a set of injection point configurations to the store. This function
   * uses std::map::insert to insert all elements of the input map into the
   * existing store. Overriding of existing entries is consistent with general
   * std::map insert operations.
   */
  void addInjectionPoints(
      std::map<std::string, std::vector<TestConfig>>& injectionPoints);

  // Register Injection point. JsonStr must be json that validates against the injection
  // point schema OR an array of objects that individually validate against that same
  // schema.
  void registerInjectionPoint(std::string json_str);


  void logInjectionPoint(std::string package, std::string name, va_list args);

  json getInjectionPointRegistrationJson(std::string name);
  /**
   * @brief getInjectionPointStore
   * @return The InjectionPointStore
   *
   * static initializer for the InjectionPointStore. This function creates a
   * static store on first call, returning that object on each subsequent call.
   */
  static InjectionPointStore& getInjectionPointStore();

  /**
   * @brief print out injection point infomration.
   */
  void print();


};  // end InjectionPointStore

}  // namespace VnV
#endif // INJECTIONPOINTSTORE_H
