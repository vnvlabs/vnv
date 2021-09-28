

#include <iostream>

#include "VnV.h"

/**
    Confirm.
**/
INJECTION_TEST(VNVPACKAGENAME, confirm) {
  nlohmann::json schema = R"(
   {
      "type" : "object",
      "properties" : {
          "continue" : {
              "type" : "boolean"
          },
          "message" : {
              "type" : "string"
          }
      },
      "required" : ["continue", "message"]
   }
   )"_json;

  nlohmann::json response;
  if (engine->Fetch("my", schema, 1000, response)) {
    std::cout << response["message"].get<std::string>() << std::endl;
  } else {
    std::cout << "FAILED" << std::endl;
  }
  
  return SUCCESS;

}
