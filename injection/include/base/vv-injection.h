
#ifndef VV_INJECTION_POINTS_H
#define VV_INJECTION_POINTS_H

#include <stdarg.h>

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>
/**
 * \file Header file for the InjectionPoint and InjectionPointStore classes.
 */

/**
 * \namespace VnV
 * All VnV objects reside in the VnV namespace.
 */
namespace VnV {

// Forward delclare
class TestConfig;  // defined in VnV-Intefaces.h

class ITest;  // defined in VnV-Interfaces.h

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
typedef std::map<std::string, std::pair<std::string, void*>> NTV;

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
  std::string
      m_scope; /**< The name of the Injection point. (TODO change name) */
  std::vector<std::shared_ptr<ITest>>
      m_tests; /**< Vector of tests given to this injection point */

  /**
   * @brief unpack_parameters
   * @param[out] ntv The map populated by this function.
   * @param[in] argp The va_list obtained from a variadic function call.
   *
   * Unpack parameters takes a va_list and parses it into a NTV (std::map)
   * object. Because all type information is lost from a va_list, the function
   * works under the assumption that the parameters are doubles of the form
   * (string, void*)  where string is a string representing the class name of
   * the object pointed to by the void* pointer. The last parameter in the
   * va_list should be a string "__VV_PARAMETERS_END__"
   */
  void unpack_parameters(NTV& ntv, va_list argp);

  /**
   * @brief addTest Add a test Config to that injection point.
   * @param c
   */
  void addTest(TestConfig c);

 public:
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
  InjectionPoint(std::string scope);

  /**
   * @brief getScope
   * @return the name of this injection point
   *
   * Get the unique name for this injection point.
   **/
  std::string getScope() const;

  /**
   * @brief runTests Run all tests at a given stage (ipType).
   * @param ipType The stage value for the injection point.
   * @param argp The va_list of parameters passed to the injection point
   *
   * Here, we run all tests in the injection point at the given stage. The stage
   *id gets passed onto the tests to ensure the correct tests are run during
   *each stage.
   **/
  void runTests(int ipType, va_list argp);

  /**
   * @brief hasTests
   * @return ( tests.size() > 0 )
   *
   * Returns true if the injection point has tests defined.
   */
  bool hasTests();
};  // end InjectionPoint.

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
  std::shared_ptr<InjectionPoint> newInjectionPoint(std::string key);

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
  std::shared_ptr<InjectionPoint> getInjectionPoint(std::string key, int stage);

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

#endif  // include gaurd.
