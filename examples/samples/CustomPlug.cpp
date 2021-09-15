
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

/**
 *  This plug takes a parameter "x", squares it and stores the result in
 *  parameter y.
 *
 *
 */
INJECTION_PLUG(Samples, samplePlug, double x, double y) {
  const double& x = get<double>("x");

  // We can get a reference to the variable "y" by calling getReference.
  // This is a reference to the raw value passed at the plug and mapped
  // to y using the input file. If we change this, we change the value
  // in the code.
  double& y = getReference<double>("y");
  y = x * x;

  // Plugs are a special case of a ITest. So, you can also do anything you
  // can with a test, including defining a runner, providing a schema, and
  // writining output using the engine
  engine->Put("y", y);

  // Return true because we successfully plugged the gap.
  return true;
}
