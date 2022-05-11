
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

/**
 *  This plug takes a parameter "x", squares it and stores the result in
 *  parameter y.
 *
 *
 */
INJECTION_PLUG(VnVPlugin, samplePlug) {
  auto x = getInputRef<double>("x", "double");
  auto y = getOutputRef<double>("y", "double");

  // Plugs are a special case of a ITest. So, you can also do anything you
  // can with a test, including defining a runner, providing a schema, and
  // writining output using the engine
  engine->Put("y", y);

  // Return true because we successfully plugged the gap.
  return true;
}
