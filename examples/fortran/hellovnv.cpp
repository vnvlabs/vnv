#include "VnV.h"

/**
 * The fortran example. 
 * 
 * 
 */
INJECTION_EXECUTABLE(fortran)

/*
 * My Custom Test
 * --------------
 *
 * The value of x was :vnv:`x`
 *
 */
INJECTION_TEST_RS(fortran, plot_something, int, R"({
    "type":"object",
    "parameters":{
        "parameter":{
            "type":"string"
         },
         "type" : {
             "type" : "string" , 
             "enum" : ["integer","float","double" ]
         } 
    }
  }
)") {
  
  std::string s = m_config.getAdditionalParameters().value("parameter","x");
  std::string t = m_config.getAdditionalParameters().value("type","float");
  if (t.compare("double")==0) {
      auto x = GetRef(s, double);
      engine->Put("x", x);
  } else if (t.compare("integer")==0 ) {
      auto x = GetRef(s, int);
      engine->Put("x", x);    
  } else {
      auto x = GetRef(s, float);
      engine->Put("x", x);
  }
  
  return SUCCESS;
}