#include "python/PythonInterface.h"

#include "base/Runtime.h"
#include "base/stores/OutputEngineStore.h"
namespace VnV {
namespace Python {

ReaderWrapper::ReaderWrapper(std::string filename) : ReaderWrapper(filename, "{}") {}

ReaderWrapper::ReaderWrapper(std::string filename, std::string config) {
  try {
    json conf = json::object();

    auto idx = filename.rfind(".");

    if (idx != std::string::npos) {
      std::string ext = filename.substr(idx + 1);
      rootNode = VnV::OutputEngineStore::instance().readFile(filename, ext, conf, true);
    } else {
      throw INJECTION_EXCEPTION("Engine Detection Failed because output file %s has no extension.", filename.c_str());
    }
  } catch (std::exception &e) {
    throw INJECTION_EXCEPTION("Engine Detection Failed because output file %s has no extension.", filename.c_str());
  }
}

ReaderWrapper::ReaderWrapper(std::string filename, std::string reader, std::string config, bool async) {
  // Reset the engine manager, using the provided reader and config.
  try {
    json conf = json::parse(config);
    rootNode = VnV::OutputEngineStore::instance().readFile(filename, reader, conf, async);
  } catch (std::exception &e) {
    throw INJECTION_EXCEPTION("Engine Detection Failed: %s %s ", filename.c_str(), e.what());
  }
}

Nodes::IRootNode* ReaderWrapper::get() { return rootNode.get(); }

void VnVFinalize() {
  try {
    RunTime::instance().Finalize();
  } catch (std::exception &e) {
    std::cout << "Finalization failed" << std::endl;
  }
}

void VnVLoadPlugin(std::string name, std::string filename) {
  try {
    RunTime::instance().loadPlugin(filename, name);
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Loading Plugin Failed: %s:%s", name.c_str(), filename.c_str());
  }
}

std::string VnVDumpReaders() {
  try {
    return OutputEngineStore::instance().listReaders().dump();
  } catch (std::exception &e) {
    VnV_Error(VNVPACKAGENAME, "Listing readers failed");
    return "[]";
  }
}

namespace {
/**
 * @brief stringsToChars Utility to convert a vec of strings to a vec of char*
 * @param vec
 * @return
 */
std::vector<char*> stringsToChars(std::vector<std::string>& vec) {
  std::vector<char*> cstrings;
  cstrings.reserve(vec.size());

  for (auto& s : vec) {
    cstrings.push_back(&s[0]);
  }
  return cstrings;
}

}  // namespace

bool VnVInit_Str(std::vector<std::string> args, std::string configStr) {
  json conf = json::parse(configStr);
  int argc = args.size();

  std::cout << " S " << args[0] << std::endl;

  std::vector<char*> cstrings = stringsToChars(args);

  char** argv = cstrings.data();
  for (int i = 0; i < argc; i++) {
    std::cout << args[i] << " " << argv[i] << std::endl;
  }

  try {
    bool success = RunTime::instance().InitFromJson("PYTHON_READER", &argc, &argv, conf, nullptr);
    return success == 0;
  } catch (std::exception &e) {
    return false;
  }
}

bool VnVInit(std::vector<std::string> args, std::string config) {
  int argc = args.size();
  std::vector<char*> cstrings = stringsToChars(args);
  char** argv = &cstrings[0];
  try {
    return RunTime::instance().InitFromFile("PYTHON_READER", &argc, &argv, config, nullptr);
  } catch (std::exception &e) {
    return false;
  }
}

bool VnVIsInitialized() {
  try {
    return RunTime::instance().isInitialized();
  } catch (std::exception &e) {
    return false;
  }
}

}  // namespace Python
}  // namespace VnV
