#ifndef VV_RUNTIME_HEADER
#define VV_RUNTIME_HEADER

/**
  \file VV-Runtime class.
  */

#include <stdarg.h>

#include <string>

/**
 * VnV Namespace
 */
namespace VnV {

/**
 * @brief The RunTime clas
 *
 * The runtime class is the (semi) user facing class of the library. This class
 * handles running of injection points in libraries, as well as initialization
 * and finalization of the VnV Objects and Engines.
 *
 * Like the EngineStore, the RunTime class uses a static initialization function
 * and a private constructor. This makes it impossible to create more than one
 * instance of the RunTime Class.
 */
class RunTime {
 private:
  /**
   * @brief RunTime
   *
   * Private Constructor -- Called used the getRunTime() function.
   *
   */
  RunTime();

  int* argc;    /**< Store the input parameters -- no idea why TODO*/
  char*** argv; /**< Stored args list for command line */

  bool runTests; /**< Should tests be run */
  bool finalize_mpi =
      false; /**< Are we responsible for calling MPI_Finalize) */

 public:
  /**
   * @brief Init
   * @param argc The initialization arguements provided to the command line.
   * @param argv The command line args list
   * @param configFile The configuration file
   * @return true is initialization was successfull.
   *
   * Initialize the Runtime environment. This includes parsing and validating
   * the input file, loading the additional libraries, populating the test
   * store, selecting and configuring the IO Engine, etc.
   *
   * At the end of this function, a single stage INJECTION_POINT is included.
   * This injection point allows the user to add tests for writing input
   * information. The Provenance test included in the tests/provenance is
   * designed to work with this injection point in mind.
   */
  bool Init(int* argc, char*** argv, std::string configFile);

  /**
   * @brief injectionPoint
   * @param injectionIndex The stage of the injection point to be run
   * @param scope The name of the injection point to be run
   * @param function The name of the function from which this injection point
   * was called.
   * @param argp The va_list containing all the arguements supplied at the
   * injection point.
   *
   * This function is called (eventually) whenever an INJECTION_POINT is found.
   * Here, we are responsibe for pulling the correct IP from the IPStore, and
   * running it with the given parameters.
   */
  void injectionPoint(int injectionIndex, std::string scope,
                      std::string function, va_list argp);

  /**
   * @brief injectionPoint
   * @param injectionIndex The stage of the injection point
   * @param scope The name of the injection point
   * @param function The function that called this injection point
   *
   * The variadic form of the injectionPoint function above.
   * This function is called (eventually) whenever an INJECTION_POINT is found.
   * Here, we are responsibe for pulling the correct IP from the IPStore, and
   * running it with the given parameters.
   *
   *
   */
  void injectionPoint(int injectionIndex, std::string scope,
                      std::string function, ...);

  /**
   * @brief Finalize
   * @return
   *
   * Finalize the RunTime Environment. This includes finalizing the
   * Engine, and, if we initialized it, finalizing MPI.
   *
   */
  bool Finalize();

  /**
   * @brief isRunTests
   * @return True if tests should be run
   *
   * Should tests be run (as configured by the user input file).
   *
   */
  bool isRunTests();

  /**
   * @brief instance
   * @return
   *
   * Get an instance of the RunTime class (only one exists).
   */
  static RunTime& instance();

  /**
   * @brief loadInjectionPoints
   * @param json
   *
   * Load injection points from a json file. This is a pooly named WIP.
   *
   * The idea here is that we can load additional injection points at any time.
   * At the moment this is used by the unit testers to add injection point
   * configuraitons dynamically.
   *
   *
   */
  static void loadInjectionPoints(std::string json);

  /**
   * @brief runUnitTests
   *
   * Run all user configured unit testers.
   */
  void runUnitTests();
};
}  // namespace VnV

extern "C" {
/**
 * @brief VnV_injectionPoint
 * @param stageVal The stage of this injection point
 * @param id The id of the injection point
 * @param function The name of the function calling this injection point
 *
 * @arg Args The parameters of the injection point. These should be pairs of
 * string,void* where string is the class name, and void* is a pointer to a
 * class of that type. The final arguement should always be a string
 * "__VV_PARAMETERS_END__"
 *
 */
void VnV_injectionPoint(int stageVal, const char* id, const char* function,
                        ...);

/**
 * @brief VnV_init
 * @param argc argc from the command line ( used in case of MPI_Init )
 * @param argv argv from the command line ( used in case of MPI_Init )
 * @param filename The configuration file name
 * @return todo.
 *
 * Initialize the VnV library. If this function is not called, no injection
 * point testing will take place.
 */
int VnV_init(int* argc, char*** argv, const char* filename);
/**
 * @brief VnV_finalize
 * @return todo
 *
 * Calls RunTime::instance().Finalize();
 */
int VnV_finalize();

/**
 * @brief VnV_runUnitTests
 * @return tod
 *
 * Calls RunTime::instance().runUnitTests().
 */
int VnV_runUnitTests();
}

#endif
