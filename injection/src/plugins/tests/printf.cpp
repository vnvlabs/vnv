

#include <iostream>

#include "VnV.h"

namespace {
static std::string default_message = "Here";
static std::string default_type = "Put";
}  // namespace

/**
  PRINT TEST: The message is TODO.
**/
INJECTION_TEST(printMessage) {
  const json& j = getConfigurationJson();
  auto typeA = j.find("type");
  auto message = j.find("message");
  std::string t =
      (typeA == j.end()) ? default_type : typeA.value().get<std::string>();
  std::string m = (message == j.end()) ? default_message
                                       : message.value().get<std::string>();

  if (t == "Put")
    engine->Put(comm, "message", m);
  else if (t == "stdout")
    std::cout << m << std::endl;
  else if (t == "Warn")
    VnV_Warn_MPI(comm, m.c_str());
  else if (t == "Debug")
    VnV_Debug_MPI(comm, m.c_str());
  else if (t == "Info")
    VnV_Info_MPI(comm, m.c_str());
  else if (t == "Error")
    VnV_Error_MPI(comm, m.c_str());
  else {
    VnV_Log_MPI(comm, t.c_str(), m.c_str());
  }
  return SUCCESS;
}
