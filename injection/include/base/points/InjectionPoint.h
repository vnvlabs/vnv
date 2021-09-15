﻿
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

/**
 * @brief Wrapper class to support C and C++ style callback functions
 * within a single class
 */
class VnVCallBack {
 private:
  DataCallback cppCallback = nullptr;         /**< Callback if callbackType is 0 */
  injectionDataCallback* cCallback = nullptr; /**< Function pointer to a C style callback. */

 public:
  /**
   * @brief Construct a new Vn V Call Back object for a Cpp callback type
   *
   * @param callback The Cpp callback .
   */
  VnVCallBack(DataCallback callback) : cppCallback(callback) {}

  /**
   * @brief Construct a new Vn V Call Back object for a C callback type
   *
   * @param callback The C callback (function pointer).
   */
  VnVCallBack(injectionPointCallback* callback) : cCallback(callback) {}

  /**
   * @brief Call the callback
   *
   * @param comm The communicator defined at the injection point
   * @param wrapper  The output engine manager
   * @param parameterMap The parameter map for the injection point
   * @param type The type of the current injection point
   * @param stageId The stage id for the the current injection point.
   */
  void call(ICommunicator_ptr comm, OutputEngineManager* wrapper, std::map<std::string, VnVParameter> parameterMap,
            InjectionPointType type, std::string stageId) {
    if (cCallback != nullptr) {
      IOutputEngineWrapper engineWraper = {static_cast<void*>(wrapper->getOutputEngine())};
      ParameterSetWrapper paramWrapper = {static_cast<void*>(&parameterMap)};
      int t = InjectionPointTypeUtils::toC(type);
      (*cCallback)(comm->asComm(), &paramWrapper, &engineWraper, t, stageId.c_str());
    } else if (cppCallback != nullptr) {
      cppCallback(comm->asComm(), parameterMap, wrapper, type, stageId);
    } else {
    }
  };

  // Typedefs
  typedef std::map<std::string, std::pair<std::string, void*>> NTV;

  /**
   * @brief The InjectionPoint Base class
   *
   * The InjectionPointBase class manages the execution of the tests at runtime. Each
   * time the INJECTION_POINT(...) macro is found in a code, the RunTime system
   * creates a new InjectionPoint object, and populates it with a list of tests to
   * execute (as specified in the input file).
   *
   * A new injection point is created for each INJECTION_POINT call in the source
   * code. The injection points are set up based on the user supplied input
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

    std::map<std::string, VnVParameter> parameterMap; /**< The parameters available at this injection point */

    //@todo define a wrapper class for these two callback types.
    std::unique_ptr<VnVCallback> callback = nullptr

        bool skipped = false; /**< Was this injection point skipped due to a sampler */

    /**
     * @brief Run the tests assigned to the injection point.
     *
     * @param wrapper The OutputEngineManager to pass to the tests.
     */
    virtual void runTestsInternal(OutputEngineManager* wrapper);

    /**
     * @brief Construct a new Injection Point Base object
     *
     * @param packageName The name of the package (unique across pacakges)
     * @param name  The name of the injection point (unique within a package)
     * @param registrationJson The registration information for the injection point
     * @param in_args The parameters passed as "input" arguments
     * @param out_args The parameters passed as output arguments.
     */
    InjectionPointBase(std::string packageName, std::string name, json registrationJson, const NTV& in_args,
                       const NTV& out_args);

    /**
     * @brief Set the Injection Point Type object
     *
     * @param type
     * @param stageId
     */
    void setInjectionPointType(InjectionPointType type, std::string stageId);

    /**
     * @brief Set the Call Back object.
     *
     * @tparam T The type of callback to set
     * @param callback
     */
    template <typename T> void setCallBack(T callback) { this->callback.reset(new VnVCallback(callback)); }

    /**
     * @brief Set the Comm object
     *
     * @param comm The communicator provided when the injection point was initialized
     */
    void setComm(ICommunicator_ptr comm);

   public:
    bool runInternal = false; /**< should we run the internal injection point test. */

    /**
     * @brief getScope
     * @return the name of this injection point
     *
     * Get the unique name for this injection point.
     *
     * @todo rename this to getName() Why in the world did I call it getScope() anyway?
     **/
    std::string getScope() const;

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
    virtual void run(std::string filename, int line) = 0;

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
   protected:
    /**
     * @brief Construct a new Injection Point object
     *
     * @param packageName The name of the package (unique across pacakges)
     * @param name  The name of the injection point (unique within a package)
     * @param registrationJson The registration information for the injection point
     * @param in_args The parameters passed as "input" arguments
     * @param out_args The parameters passed as output arguments.
     */
    InjectionPoint(std::string packageName, std::string name, json registrationJson, const NTV& in_args,
                   const NTV& out_args)
        : InjectionPointBase(packageName, name, registrationJson, in_args, out_args){};

   public:
    /**
     * @brief Construct a new Injection Point object
     *
     * Injection points (single and looped) do not have out_args. This constructor
     * hides that option from the API.
     *
     * @param packageName The name of the package (unique across pacakges)
     * @param name  The name of the injection point (unique within a package)
     * @param registrationJson The registration information for the injection point
     * @param in_args The parameters passed as "input" arguments
     */
    InjectionPoint(std::string packageName, std::string name, json registrationJson, NTV& in_args)
        : InjectionPointBase(packageName, name, registrationJson, in_args, {}){};

    /**
     * @copydoc VnV::InjectionPointBase::run
     */
    virtual void run(std::string function, int line) override;
  };

}  // namespace VnV

#endif  // include gaurd.
