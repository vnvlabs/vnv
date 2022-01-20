
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

// Register your executable
INJECTION_EXECUTABLE(Samples)

int main(int argc, char** argv) {
  double x(0), y(0), z(0), w(0);

  // Initalize The VnV Library
  INJECTION_INITIALIZE(Samples, &argc, &argv, "input.json");

  // Logging -- Default it to only write logs on the root comm of the world
  // processor.
  VnV_Debug(Samples, "Hello this is a log %s", "message");
  VnV_Warn(Samples, "Hello this is a log %s", "message");
  VnV_Error(Samples, "Hello this is a log %s", "message");
  VnV_Info(Samples, "Hello this is a log %s", "message");

  // If you explicitly list the communicator, then it will write on rank 0
  // of that communicator.

  // Write from all processors (self comm)
  VnV_Debug_MPI(Samples, VSELF, "This is a log message %d", 10);

  // Write on world rank 0 (same as if you left out the comm)
  VnV_Warn_MPI(Samples, VWORLD, "This is a log message %d", 10);

  // Write on rank 0 of some custom comm.
  // Commented because we dont link mpi in this example.
  // MPI_Comm comm;
  // VnV_Info_MPI(Samples, VMPI(comm), "This is a log message %d", 10);

  /**
   * My First Injection Point
   * ------------------------
   *
   * Injection points let you define points in the code where users can perform
   * tests on a variable. An injection point includes four parts;
   *
   *   1. The Package name. This is the name of the package that places the
   * injection point. In this case, we called our package "ADD_INJECTION_POINT".
   *   2. The Communicator. The communicator defines the group of processors
   * that will call this injection point. You can call VSELF, VWORLD or
   * VMPI(MPI_Comm). If this is a serial application you should use VSELF. The
   * injection point is collective across this communicator.
   *   3. The Injection Point name. The name of the injection point is used to
   * identify the injection point in reports and input files. It should be a
   * string literal that is unique within the package.
   *   4. The parameter list. The parameter list is a list of parameters that
   * you want to make available at the injection point. Injection tests will be
   * able to access these parameters for analysis purposes. In this case, we
   * pass the parameter argc.
   *   5. The comment. The comment is a c++ comment block located directly above
   * the injection point. The comment is a Restructured text template that will
   * be rendered with each instance of the injection point in the final report.
   * That is to say, this text will appear in the final report.
   */
  INJECTION_POINT("ADD_INJECTION_POINT", VSELF, "MY_FIRST_INJECTION_POINT",
                  argc);

  /**
   * My Second Injection Point
   * ------------------------
   *
   * Injection points also let you define a callback routine. To add a callback,
   * you should use the INJECTION_POINT_C call.
   *
   * Inside the callback, you have access to five main parameters.
   *
   * 1. The communication object (comm)
   * 2. The parameter set
   * 3. The output engine,
   * 4. The type of injection point
   *
   * The callback will always be executed each time the injection point call is
   * found. The callback can be used to hard code tests into injection points.
   *
   * As we will outline soon, the callback can also be used to write data that
   * can be used in the template during rendering. For instance, this callback
   * writes the value of argc and uses it when rendering the following.
   *
   * .. note:: The number of arguments passed to the executable was
   * :vnv:`data.argc.value`.
   *
   * We will explain more about writing data and how to reference data in the
   * templates soon.
   *
   * */
  INJECTION_POINT_C(
      "Samples", VSELF, "MY_FIRST_INJECTION_POINT_CALLBACK",
      IPCALLBACK {
        int a = ntv.GetRef("argc",int); 
        engine->Put("argc", a);
      },
      argc);

  /**
   * My First Injection Point Loop
   *
   * Loops let you monitor a region of code. Variables must be valid scope
   * across the entire loop. All tests will have a lifetime equal to the
   * lifetime of the loop. Loops should have a begining and and end statement,
   * any number of ITER statements are aloud.
   */
  int i = 0;
  INJECTION_LOOP_BEGIN("Samples", VSELF, "MY_FIRST_INJECTION_LOOP", i);
  for (int i = 0; i < 5; i++) {
    INJECTION_LOOP_ITER("Samples", "MY_FIRST_INJECTION_LOOP", "INSIDE");
  }
  INJECTION_LOOP_END("Samples", "MY_FIRST_INJECTION_LOOP");

  /**
   * Plugs let you define a region of code that can be plugged with another
   * function. The Plug syntax is:
   *
   *  Parameter 1: Var -- This is a valid variable name that is available within
   *  the current scope. The Plug macro generates an iterator object that will
   * be stored in this variable to keep track of the Plug process.
   *
   *  Parameter 2: Package Name
   *  Parameter 3: Communicator
   *  Parameter 4: Unique Plug name
   *  Parameter 5: The number of input parameters
   *  Parameter 6->6+P5 The input parameters
   *  Parameter 6+P5 -> 25: The output parameters that must be set by the plug.
   *
   *  The body of the Function plug is the default plug. This will be called in
   *  cases where an alternative plug is not provided.
   *
   *  In this case, we have defined a plug called "squareme" that
   *  has "1" input parameter (x) and expects 1 output parameter z.
   *
   */
  INJECTION_FUNCTION_PLUG(a, "Samples", VSELF, "squareme", 1, x, z) {
    z = x * x;
  }

  /**
   *  Iterators let you define a region of code that can be iterated over by
   *  an iterator.
   *
   *
   *  Parameter 1: Var -- This is a valid variable name that is available within
   *  the current scope. The Plug macro generates an iterator object that will
   * be stored in this variable to keep track of the Plug process.
   *
   *  Parameter 2: Package Name
   *  Parameter 3: Communicator
   *  Parameter 4: Unique Plug name
   *  parameter 5: The minimum number of times to run the body when no iterator
   * is supplied. Parameter 6: The number of input parameters Parameter 7->7+P5
   * The input parameters Parameter 7+P5 -> 25: The output parameters that must
   * be set by the plug.
   *
   *  The body of the Function plug is the default plug. This will be called in
   *  cases where an alternative plug is not provided.
   *
   *  In this case, we have defined an iteration point that sets the output
   * parameter (z) equal to the input parameter "x".
   *
   *  In this case we have set "once" to 1. If the users input file does not map
   * an iterator to this iteration point, the body of the iteration point will
   * be executed one time.
   *
   */
  INJECTION_ITERATION(b, "Samples", VSELF, "equal", 1, 1, x, z) { z = x; }

  // Finalize the VnV Library
  INJECTION_FINALIZE(ADD_INJECTION_POINT)

  return 0;
}