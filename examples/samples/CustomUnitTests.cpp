
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

// The toolkit has a built in unit testing framework. Tests can have any number of processors
// involved in them. The last parameter to the INJECTION_UNITTEST macro is the number of cores
// VnV will only run tests where we have enough cores to do so. When running unit tests, VnV 
// creates custom sub-communicators to execute the tests. A communication packing algorithm is 
// used to parallelize the execution of unit tests where ever possible. 

/**
 * The unit test comment. This is rendered in the final report. 
 */ 
 INJECTION_UNITTEST(Samples, test1, 1) {

  /** The test assert comment will be rendered in the report **/  
  TEST_ASSERT_EQUALS("true", true, true);

  /** Format is "name", "expected", "got" **/
  TEST_ASSERT_EQUALS("false", false, false);
  
  
  TEST_ASSERT_EQUALS("not true", !true, false);
 }


/**
 * Parallel unit test
 * 
 */ 
 INJECTION_UNITTEST(Samples, test2, 2) {

  int worldSize = comm->Size();

  /** The world size should be two. **/  
  TEST_ASSERT_EQUALS("world Size", 2, worldSize);

 }
