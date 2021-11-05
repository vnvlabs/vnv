
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

class run {
 public:
  int x = 0;
};

// Define a custom Iterator
/**
 *  Iterators are designed to allow iterative algorithms like UQ/SA/ parameter
 * optimization. The idea is that the iterator sets the input parameters and
 * analyzes the output parameters.
 *
 *  The process is:
 *
 *  1. VnV calls the iterator asking it to set the input parameters.
 *  2. The iterator sets the input parameters
 *  3. The body of the iteration point is executed using the input parameters.
 *  4. VnV calls the iterator asking it to set the next set of input parameters;
 *  5. The iterator looks at the updated output parameters and decides on the
 * next set of input parameters. 6 ...... repeat .....
 *
 *  n: The iterator returns a 0 when it is done. The body of the iteration point
 * will not be executed again. The output parameters will remain as is.
 *
 *  This silly little iterator adds one to to input parameter until the output
 * parameter is greater than 10. In a real iterator, you would want to put some
 * sort of tolerance such that it cannot run forever.
 *
 *  Iterators are a special type of test -- You can do anything you can do in a
 * test in a iterator. The comment above will be rendered in the final report
 * (thats this comment). You can write data using the engine.
 *
 */
INJECTION_ITERATOR_R(Samples, sampleIterator, run) {
  
  auto y = getOutputRef<double>("y","double");
  
  if (y < 10) {
    double& x = getInputRef<double>("x", "double");
    x += 1;
    return 1;
  }
  return 0;
}
