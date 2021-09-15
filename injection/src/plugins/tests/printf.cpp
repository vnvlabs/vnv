

#include <iostream>

#include "VnV.h"

namespace {
static std::string default_message = "Here";
static std::string default_type = "Put";
}  // namespace

/**
  .. (comment) vnv-process is the best way to handle nested parsing.
     Basically "mess" will be substituted in for $$mess$$. This
     allows for a message that contains restructured text to be
     processed properly.

  .. vnv-process::
    :mess: `Data.message`

    $$mess$$
**/
INJECTION_TEST(VNVPACKAGENAME, printMessage) {
  const json& j = getConfigurationJson();
  auto message = j.find("message");
  std::string m = (message == j.end()) ? default_message
                                       : message.value().get<std::string>();
  engine->Put("message", m);
  return SUCCESS;
}
