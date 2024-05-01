
/**
 * @file Header file for the InjectionPoint and InjectionPointStore classes.
 */

#ifndef VV_INJECTION_POINTS_H
#define VV_INJECTION_POINTS_H

#include <stdarg.h>

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <list>


#include "c-interfaces/Wrappers.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/ISampler.h"
#include "interfaces/ITest.h"
#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {

// Forward declare important classes
class InjectionPointStore;
class TestConfig;
class IIterator;
class IPlug;
class VnVParameter;
class RunTime;
class OutputEngineManager;
enum class InjectionPointType;
enum class InjectionPointType;

// Typedefs
typedef std::map<std::string, std::pair<std::string, void*>> NTV;

typedef std::function<void(std::string&, std::size_t&, std::list<std::string, std::string>&)> templateFunc;

/**
 * @brief The InjectionPoint Base class
 *
 * The InjectionPointBase class manages the execution of the tests at runtime. Each
 * time the INJECTION_POINT(...) macro is found in a code, the RunTime system
 * creates a new InjectionPoint object, and populates it with a list of tests to
 * execute (as specified in the input file).
 *
 * A new injection point is created for each INJECTION_POINT call in the source
 * code. The injection points are set up based on the user-supplied input
 * configurations stored in the injection point store.
 *
 */
class InjectionPointBase {
 protected:
  friend class InjectionPointStore;  // Injection point store is manager
  friend class RunTime;              // Runtime can access

  static constexpr const char* internalTestName = "__internal__";

  ICommunicator_ptr comm; /**< The communicator defined for this injection point */

  //@todo move to the IteratorPoint derived class

  std::string name;    /**< name of this injection point */
  std::string package; /**< Package that made this injection point call */

  std::shared_ptr<ISampler> sampler = nullptr; /**< Sampler used to decide when injection point should be exuected*/

  std::vector<std::shared_ptr<ITest>> m_tests; /**< Vector of tests given to this injection point */

  InjectionPointType type; /**< The type of injection point */
  std::string stageId;     /**< The stage Id of the injection point  */

  VnV::VnVParameterSet parameterMap; /**< The parameters available at this injection point */

  bool skipped = false; /**< Was this injection point skipped due to a sampler */

  /**
   * @brief Run the tests assigned to the injection point.
   *
   * @param wrapper The OutputEngineManager to pass to the tests.
   */
  virtual void runTestsInternal(OutputEngineManager* wrapper, const DataCallback& callback);

  /**
   * @brief Construct a new Injection Point Base object
   *
   * @param packageName The name of the package (unique across pacakges)
   * @param name  The name of the injection point (unique within a package)
   * @param registrationJson The registration information for the injection point
   * @param args The parameters passed as "input" arguments
   */
  InjectionPointBase(std::string packageName, std::string name, std::map<std::string, std::string> registrationJson,
                     const NTV& args);

  /**
   * @brief Set the Injection Point Type object
   *
   * @param type
   * @param stageId
   */
  void setInjectionPointType(InjectionPointType type, std::string stageId);

  /**
   * @brief Set the Comm object
   *
   * @param comm The communicator provided when the injection point was initialized
   */
  void setComm(ICommunicator_ptr comm);

 public:
  bool runInternal = false; /**< should we run the internal injection point test. */

  std::string getName() const { return name; }

  std::string getPackage() const { return package; }

  /**
   * @brief addTest Add a test Config to the injection point.
   *
   * Note: This function simply adds the test. It does not validate it. It is a private
   * function only called by the InjectionPointStore (friend class).
   *
   * @param c
   */
  void addTest(TestConfig& c);

  /**
   * @brief Get the RTTI for a parameter if we have it (C++ InjectionPoints only).
   *
   * @note RTTI information is compiler specific -- we should never rely on it across
   * instantiations?
   *
   * @param key The name of the parameter to fetch RTTI for.
   * @return The RTTI string defined for the parameter
   */
  std::string getParameterRTTI(std::string key) const;

  /**
   * @brief Run the injection point.
   *
   * This is called after the injection point stage and type have been set.
   *
   * @param function The name of the file containing this injection point
   * @param line The line of the injection point in that file.
   *
   * @todo Injection point stage and type should be required here. Requiring they be set before
   * this call is a bug waiting to happen.
   */
  virtual void run(std::string filename, int line, const DataCallback& callback) = 0;

  /**
   * @brief Destroy the Injection Point Base object
   */
  virtual ~InjectionPointBase(){};

};  // end InjectionPoint.

/**
 * @brief Derived class for handling single and looped injection points.
 *
 */
class InjectionPoint : public InjectionPointBase {
 public:
  /**
   * @brief Construct a new Injection Point object
   *
   * @param packageName The name of the package (unique across pacakges)
   * @param name  The name of the injection point (unique within a package)
   * @param registrationJson The registration information for the injection point
   * @param args The parameters passed as "input" arguments
   */
  InjectionPoint(std::string packageName, std::string name, std::map<std::string, std::string> registrationJson,
                 const NTV& args)
      : InjectionPointBase(packageName, name, registrationJson, args){};

  /**
   * @copydoc VnV::InjectionPointBase::run
   */
  virtual void run(std::string function, int line, const DataCallback& callback) override;
};

}  // namespace VnV

#endif  // include gaurd.
