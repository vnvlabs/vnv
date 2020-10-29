#ifndef OFFLOADERSTORE_HPP
#define OFFLOADERSTORE_HPP

#include <map>
#include <string>

#include "interfaces/IOffloader.hpp"

/**
  \file Headers for the Offloader Store.
*/

/**
 * VnV Namespace
 */
namespace VnV {

/**
 * @brief The Offloader Store class
 *
 */

class OffloaderStore {
 private:
  std::map<std::string, offloader_register_ptr*> registeredOffloaders; /**< List of all registered engines */
  std::shared_ptr<IOffloader> manager; /**< The current Engine Manager being used in VnV for all IO */
  std::string offloaderName;
  /**
   * @brief EngineStore
   * Private constructor called through public static function getEngineStore()
   */
  OffloaderStore();


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
  void registerOffloader(std::string name, offloader_register_ptr* offload_ptr);

  /**
   * @brief getEngineManager
   * @return The currently active engine manager. nullptr if not yet configured.
   *
   * Get the configured Engine Manager.
   */
  std::shared_ptr<IOffloader> getOffloader();


  std::shared_ptr<IOffloader> setOffloader(std::string key, json& config);

  /**
   * @brief getEngineStore
   * @return The EngineStore.
   */
  static OffloaderStore& get();

};


}  // namespace VnV
#endif // OFFLOADERSTORE_HPP
