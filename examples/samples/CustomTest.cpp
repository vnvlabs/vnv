
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

#define PNAME Samples

// This is a set of tests. You can compile tests into a separate testing
// library, or you can add them directly in the executable. All that is
// required is that you call the INJECTION_EXECUTABLE(PackageName) macro
// at some point.

/*
 * My Custom Test
 * --------------
 *
 * The value of x was :vnv:`data.x.value`
 *
 */
INJECTION_TEST(PNAME, customTest) {
  // You can pull parameters out of the test parameter map using the get
  // function.
  auto x = GetRef("x", double);


  // You can write values to the output engine using the IOutputEngine
  // interface. These values can be used in the test comment template as shown
  // above.
  engine->Put("x", x);
  return SUCCESS;
}

/**
 * My Custom Test 2
 * --------------
 *
 * The sum of x and y was :vnv:`data.sum.value`
 */
INJECTION_TEST(PNAME, customTest2) {
  // You can have up to 20 test parameters
  auto x = GetRef("x", double);
  auto y = GetRef("y", double);
  engine->Put("sum", x + y);
  return SUCCESS;
}

// VnV uses JSON Schema to validate user input. Each test can define
// a JSON schema that should be used to validate the user provided test
// configuration options. In this case, the test will require a parameter
// called p1 that is a string.
const char* c3Schema = R"(
    {
         "type" : "object" , 
         "properties" : { 
              "p1" : {"type" : "string" }
         },
         "required" : ["p1"]
    }
)";

// You can also define a simple runner class. This allows you to organize your
// tests using OOP principles. The only requirement for a runner is that it has
// a public constructor with no parameters. When a runner is defined, you will
// have access to a shared_ptr called "runner" pointing to an instance of the
// runner class (std::shared_ptr<CRunner>)
class CRunner {
 public:
  CRunner() {}

  void run(VnV::IOutputEngine* engine) { engine->Put("x", 10); }
};

/**
 * Custom test with a runner and a schema
 * --------------------------------------
 *
 * This test uses the INJECTION_TEST_RS macro to define a test with
 * a schema and a runner. It has one test parameter.
 *
 */
INJECTION_TEST_RS(PNAME, customTest3, CRunner, c3Schema) {
  
  // Calling getConfigurationJson() returns a const reference to the user
  // provided json configuration object that has been validated against the
  // schema provided eariler. With a good schema, you can avoid checking the
  // existence and type of parameters.
  std::string configParameter = getConfigurationJson()["p1"].get<std::string>();

  // You can call any public functions of your runner class.
  runner->run(engine);

  // Return success
  return SUCCESS;
}

/**
 * Handling Injection Point Stages.
 * ==================
 *
 * Something in here I guess.
 */
INJECTION_TEST(PNAME, customTest4) {
  
  auto y = GetRef("y", double);


  // Injection points have stages. You can get the current stage using the type
  // variable
  switch (type) {
  case InjectionPointType::Begin:
  case InjectionPointType::End:

    engine->Put("y", y);
    break;

  case InjectionPointType::Iter:

    // We can also provide meta data to any Put. Metadata is a string->string
    // map of values that will be associated with the variable.
    engine->Put("y", y, {{"stageId", stageId}});
    break;

  case InjectionPointType::Single:
    return FAILURE;
  }

  return SUCCESS;
}
