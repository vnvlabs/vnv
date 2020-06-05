#include "VnV.h"
#include <signal.h>

namespace {

#define SIGS                                                                  \
  X(SIGINT)                                                                   \
  X(SIGQUIT) X(SIGILL) X(SIGABRT) X(SIGKILL) X(SIGSEGV) X(SIGPIPE) X(SIGALRM) \
      X(SIGTERM) X(SIGUSR1) X(SIGUSR2) X(SIGCHLD) X(SIGCONT) X(SIGSTOP)       \
          X(SIGTSTP) X(SIGTTIN) X(SIGTTOU) X(SIGPROF) X(SIGSYS)

#define X(name) \
  , { #name, name }

std::map<std::string, int> sigmap = {{"SIGTRAP", SIGTRAP} SIGS};
int default_signal = SIGTRAP;

#undef SIGS
#undef X
}  // namespace

INJECTION_TEST(hardBreakpoint, int* argv) {
  json confj = getConfigurationJson();
  auto it = confj.find("signal");
  if (it != confj.end()) {
    auto s = sigmap.find(it.value().get<std::string>());
    if (s != sigmap.end()) {
      raise(s->second);
    } else {
      VnV_Warn("Invalid Signal passed to breakpoint test. Ignoring");
    }
  } else {
    raise(default_signal);
  }
  return SUCCESS;
}
