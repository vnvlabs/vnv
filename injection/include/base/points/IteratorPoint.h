
/**
  @file InjectionPoint.h
**/
#ifndef VV_ITERATION_POINTS_H
#define VV_ITERATION_POINTS_H

#include "base/points/InjectionPoint.h"
#include "interfaces/IIterator.h"

namespace VnV {

/**
 * @brief Iteration implementation of the InjectionPointBase Implementation
 *
 * This class overrides the InjectionPoint interface to support Iteration
 * points within the toolkit.
 *
 * Iteration points (technically regions) allow the user to run iterative algorithms
 * across a region of code. This can be usefull for things like parameter optimization,
 * sensitivity analysis, UQ, etc.
 *
 * Users can assign any number of iterators to an iteration point. Currently all iteraters are
 * called sequentially. In the future, we will use a pub-sub algorithm to ensure multiple iterations
 * with the same parameters are not made.
 */
class IterationPoint : public InjectionPoint {
  std::vector<std::shared_ptr<IIterator>> m_iterators;
  int started = 0;
  int once = 0;    /** Minimum number of times the loop will run.  */
  int itIndex = 0; /**< Current iteration index.  */

 public:
  /**
   * @brief Construct a new Iteration Point object
   *
   * @param packageName The name of the package that defined the iteration point.
   * @param name  The name of the iteration point
   * @param registrationJson The Json describing the parameters
   * @param once_ The minimum number of times to "iterate"
   * @param args The parameters that can be modified
   */
  IterationPoint(std::string packageName, std::string name, std::map<std::string, std::string> registrationJson,
                 int once_, NTV& args)
      : InjectionPoint(packageName, name, registrationJson, args), once(once_){};

  /**
   * @brief Add an iterator to the list of iterators that will be executed at this iteration point.
   *
   * @param c The iterator config to add
   */
  void addIterator(IteratorConfig& c);

  /**
   * @brief Run the actual iteration
   *
   * @param filename The file containing the iteration point
   * @param line The line of the iteration point in that file
   * @return true We are done iterating
   * @return false We have more iterations to complete.
   */
  virtual bool iterate(std::string filename, int line);
};

}  // namespace VnV

#endif  // include gaurd.
