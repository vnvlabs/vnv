
/**
  @file InjectionPoint.h
**/
#ifndef VV_INJECTION_POINTS_H
#define VV_INJECTION_POINTS_H

#include <stdarg.h>

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "c-interfaces/Wrappers.h"
#include "json-schema.hpp"
#include "interfaces/ITest.h"
#include "interfaces/ISampler.h"
#include "interfaces/ICommunicator.h"

using nlohmann::json;
/**

 * \file Header file for the InjectionPoint and InjectionPointStore classes.
 */

/**
 * \namespace VnV
 * All VnV objects reside in the VnV namespace.
 */
namespace VnV {

// Forward declare.
class InjectionPointStore;
class TestConfig;
class IIterator;
class IPlug;
class VnVParameter;
class RunTime;
class OutputEngineManager;

enum class InjectionPointType;


typedef std::map<std::string, std::pair<std::string, void*>> NTV;


enum class InjectionPointType;

/**
 * \class InjectionPoint
 * @brief The InjectionPoint class
 *
 * The InjectionPoint class manages the execution of the tests at runtime. Each
 * time the INJECTION_POINT(...) macro is found in a code, the RunTime system
 * creates a new InjectionPoint object, and populates it with a list of tests to
 * execute (as specified in the input file).
 *
 * A new injection point is created for each INJECTION_POINT call in the source
 * code. The injection points are set up based on the user supplied input
 * configurations stored in the injection point store.
 *
 *
 */
class InjectionPointBase {
 protected:
  /**
   * InjectionPointStore manages all constructing and destructing of the
   * InjectionPoints.
   */
  friend class InjectionPointStore;
  friend class RunTime;

  static constexpr const char* internalTestName = "__internal__";

  ICommunicator_ptr comm;
  int itIndex = 0;

  std::string name;
  std::string package;

  std::shared_ptr<ISampler> sampler = nullptr;

  std::vector<std::shared_ptr<ITest>> m_tests; /**< Vector of tests given to this injection point */
  std::shared_ptr<IPlug> m_plug = nullptr; 

  InjectionPointType type;
  std::string stageId;

  std::map<std::string, VnVParameter> parameterMap;

  int callbackType = 0;
  DataCallback cppCallback = nullptr;
  injectionDataCallback* cCallback = nullptr;

  //We set this true when the sampler says we should skip. We cant just
  //throw away the data structure for heirarchy reasons, 
  bool skipped = false;

  virtual void runTestsInternal(OutputEngineManager* wrapper);  

  /**
   * @brief InjectionPoint
   * @param scope The unique name of this injection point.
   *
   * The Injection point constructor. Note that the name should be unique across
   *all injection points in the entire library. It is probably a good idea to
   *"scope" your injection points based on the function in which they lie,
   *although this is not required.
   *
   * @todo We should add some sort of naming connvention to injection point
   *names. This would allow us to organize input files in some way. E.g.,
   *support names of the form "a::b::c"
   *
   **/
  InjectionPointBase(std::string packageName, std::string name, json registrationJson, const NTV& in_args, const NTV& out_args);

  void setInjectionPointType(InjectionPointType type, std::string stageId);
  void setCallBack(injectionDataCallback* callback);
  void setCallBack(const DataCallback& callback);
  void setComm(ICommunicator_ptr comm);

 public:
   bool runInternal = false;

  /**
   * @brief getScope
   * @return the name of this injection point
   *
   * Get the unique name for this injection point.
   **/
  std::string getScope() const;

  /**
   * @brief addTest Add a test Config to that injection point. Note: This
   * function simply adds the test. It does not validate it. It is a private
   * function only called by the InjectionPointStore (friend class).
   * @param c
   */
  void addTest(TestConfig &c);


  /**
   * @brief getParameterRTTI Get the RTTI for a parameter if we have it (C++
   * INjectionPoints only).
   * @param key
   * @return
   */
  std::string getParameterRTTI(std::string key) const;
  /**
   * @brief runTests Run all tests at a given stage (ipType).
   * @param ipType The stage value for the injection point.
   * @param argp The va_list of parameters passed to the injection point
   */
  virtual void run(std::string function, int line) = 0;

  virtual ~InjectionPointBase() {};

};  // end InjectionPoint.


class InjectionPoint : public InjectionPointBase {
protected:
  // This one is just for the subclasses. 
  InjectionPoint(std::string packageName, std::string name, json registrationJson, const NTV& in_args, const NTV& out_args) 
    : InjectionPointBase(packageName, name, registrationJson, in_args, out_args) {};

public: 
  InjectionPoint(std::string packageName, std::string name, json registrationJson, NTV& in_args)
  : InjectionPointBase(packageName, name, registrationJson, in_args, {}) {};

  virtual void run(std::string function, int line) override;
};


}  // namespace VnV

#endif  // include gaurd.
