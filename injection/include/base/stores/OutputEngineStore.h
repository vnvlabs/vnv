
/**
  @file OutputEngineStore.h
**/

#ifndef VV_OUTPUTREADER_HEADER
#define VV_OUTPUTREADER_HEADER

#include <map>
#include <string>

#include "base/stores/BaseStore.h"
#include "interfaces/IOutputEngine.h"

/**
  \file Headers for the Engine Store.
*/

/**
 * VnV Namespace
 */
namespace VnV {

/**
 * @brief The EngineStore class
 * Static Engine store for managing the output Engine.
 * Follows the static initialization approach used throughout.
 *
 * Engines are used to complete the IO of the VnV tests. To add
 * an engine into the VnV library, the developer should implement
 * the OutputEngineManager Interface defined in VnV-Interfaces.h
 * and call the EngineStore::getEngineStore().registerEngine() function.
 *
 * The register engine functions requires a unique name and a function
 * pointer to a function that creates a new instance of the custom
 * Engine.
 *
 * Developers can avoid manually calling the registerEngine function
 * using the static initialization technique. See vv-debug-engine.cpp
 * for an example of this approach.
 *
 * When that approach is used, Engines are automatically registered with
 * the EngineStore when the library is loaded. This includes external
 * libraries loaded using the "testLibraries" portion of the input file. That is
 * to say, engines can be compiled in seperate .so files an imported and
 * registered with the VnV Engine store through the input file.
 *
 */
class OutputEngineStore : public BaseStore {
 private:
  std::map<std::string, engine_register_ptr>
      registeredEngines; /**< List of all registered engines */
  std::map<std::string, engine_reader_ptr>
      registeredReaders; /**< List of all registered engine readers */

  std::shared_ptr<OutputEngineManager>
      manager; /**< The current Engine Manager being used in VnV for all IO */
  std::string engineName;

  bool initialized = false; /**< Set when the Engine has been initialized */

  long idCounter = 0;

 public:
  /**
   * @brief EngineStore
   * Private constructor called through public static function getEngineStore()
   */
  OutputEngineStore();

  /**
   * @brief registerEngine
   * @param name The name of this engine.
   * @param engine_ptr The engine_ptr function that creates a new object.
   *
   * Register an engine with the EngineStore. Here, the engine_register_ptr
   * repesents a function pointer to a function that creates a new instance
   * of a child impl of OutputEngineManager.
   *
   */
  void registerEngine(std::string name, engine_register_ptr engine_ptr);

  void registerReader(std::string name, engine_reader_ptr reader_ptr);

  json listReaders();

  json schema();
  /**
   * @brief getEngineManager
   * @return The currently active engine manager. nullptr if not yet configured.
   *
   * Get the configured Engine Manager.
   */
  OutputEngineManager* getEngineManager();

  /**
   * @brief readFile
   * @param filename name of the file
   * @param engine name of the engine
   * @param config config options for the engine.
   * @return
   */
  std::shared_ptr<Nodes::IRootNode> readFile(std::string filename,
                                             std::string engine, json& config);

  /**
   * @brief setEngineManager
   * @param key The name of the engine to use in this execution of the VnV
   * library.
   * @param config Configuration parameters provided to the input file for the
   * chosen engine.
   *
   * Setup the engine manager for use based on the users provided parameters.
   * Once an engine has been created, we call engine->set(json) to allow the
   * engine to configure itself. As part of that configuration, VnV
   * automatically validates the json parameters against the engines provided
   * additionalPropertiesSchema (if one is supplied).
   *
   */
  void setEngineManager(ICommunicator_ptr world, std::string key, json& config);

  /**
   * @brief printAvailableEngines
   *
   * Print information about all available engines. This function prints
   * all info as INFO level to the Logger. If the logger is turned off for
   * that level (or removed in compilation), this function does nothing.
   */
  void printAvailableEngines();

  bool isInitialized();

  /**
   * @brief print
   */
  void print();

  static OutputEngineStore& instance();
};

}  // namespace VnV

#endif
