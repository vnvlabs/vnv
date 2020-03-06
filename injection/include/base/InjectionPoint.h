
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
#include "interfaces/IOutputEngine.h"
#include "interfaces/ITest.h"
#include "base/Runtime.h"
#include "c-interfaces/CppInjection.h"

/**

 * \file Header file for the InjectionPoint and InjectionPointStore classes.
 */

/**
 * \namespace VnV
 * All VnV objects reside in the VnV namespace.
 */
namespace VnV {

// Forward delclare

class InjectionPointStore;  // defined below.



/**
 * \typedef
 * @brief NTV
 *
 * The NTV object is passed to all ITest implementations. The structure of the
 * NTV map when recieved is <name>->pair(<type>,<ptr>) for each parameter. This
 * is intended to give the ITest implementation enough information to cast the
 * void* pointer back to its intended form.
 */

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
class InjectionPoint {
 private:
  /**
   * InjectionPointStore manages all constructing and destructing of the
   * InjectionPoints.
   */
  friend class InjectionPointStore;
  std::string  m_scope; /**< The name of the Injection point. (TODO change name) */
  std::vector<std::shared_ptr<ITest>> m_tests; /**< Vector of tests given to this injection point */

  InjectionPointType type;
  std::string stageId;

  std::map<std::string, VnVParameter> parameterMap;

  /**
   * @brief addTest Add a test Config to that injection point. Note: This function
   * simply adds the test. It does not validate it. It is a private function only
   * called by the InjectionPointStore (friend class).
   * @param c
   */
  void addTest(TestConfig c);

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
  InjectionPoint(json registrationJson, NTV &args);


public:


  /**
   * @brief getScope
   * @return the name of this injection point
   *
   * Get the unique name for this injection point.
   **/
  std::string getScope() const;

  /**
   * @brief getParameterRTTI Get the RTTI for a parameter if we have it (C++ INjectionPoints only).
   * @param key
   * @return
   */
  std::string getParameterRTTI(std::string key) const;
  /**
   * @brief runTests Run all tests at a given stage (ipType).
   * @param ipType The stage value for the injection point.
   * @param argp The va_list of parameters passed to the injection point
   *
   * Here, we run all tests in the injection point at the given stage. The stage
   *id gets passed onto the tests to ensure the correct tests are run during
   *each stage.
   **/
  void runTests();

  void setInjectionPointType(InjectionPointType type, std::string stageId);

  /**
   * @brief hasTests
   * @return ( tests.size() > 0 )
   *
   * Returns true if the injection point has tests defined.
   */
  bool hasTests();

};  // end InjectionPoint.


}  // namespace VnV

#endif  // include gaurd.
