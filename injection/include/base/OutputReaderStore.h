
/**
  @file OutputEngineStore.h
**/

#ifndef VV_OUTPUT_HEADER
#define VV_OUTPUT_HEADER

#include <map>
#include <string>
#include "interfaces/IOutputReader.h"
/**
  \file Headers for the Engine Store.
*/

/**
 * VnV Namespace
 */
namespace VnV {

typedef Reader::IReader* reader_register_ptr();
typedef Reader::ITreeGenerator* tree_gen_register_ptr();

/**
 * @brief d in seperate .so files an imported and
 * registered with the VnV Engine store through the input file.
 *
 */
class OutputReaderStore {
 private:
  std::map<std::string, reader_register_ptr*> registeredReaders; /**< List of all registered engines */
  std::map<std::string, tree_gen_register_ptr*> registeredGenerators; //List of all tree generators.

  OutputReaderStore();

  bool initialized = false; /**< Set when the Engine has been initialized */

 public:
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
  void registerReader(std::string name, reader_register_ptr* engine_ptr);
  void registerTreeGenerator(std::string name, tree_gen_register_ptr* gen_ptr);

  std::unique_ptr<Reader::IReader> getReader(std::string reader);
  std::unique_ptr<Reader::ITreeGenerator> getTreeGenerator(std::string generator);

  /**
   * @brief getEngineStore
   * @return The EngineStore.
   */
  static OutputReaderStore& instance();

  /**
   * @brief printAvailableEngines
   *
   * Print information about all available engines. This function prints
   * all info as INFO level to the Logger. If the logger is turned off for
   * that level (or removed in compilation), this function does nothing.
   */
  void printAvailableReaders();

};


void registerReader(std::string name, VnV::reader_register_ptr r);
void registerTreeGenerator(std::string name, VnV::tree_gen_register_ptr r);

}  // namespace VnV

#endif
