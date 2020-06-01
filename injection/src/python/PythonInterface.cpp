#include "python/PythonInterface.h"
#include "base/OutputEngineStore.h"
#include "base/Runtime.h"
namespace VnV {
namespace Python {

ReaderWrapper::ReaderWrapper(std::string filename) : ReaderWrapper(filename,"{}") {

}

ReaderWrapper::ReaderWrapper(std::string filename, std::string config) {

  json conf = json::object();
  auto idx = filename.rfind(".");
  if ( idx != std::string::npos) {
      std::string ext = filename.substr(idx+1);
      rootNode.reset(VnV::OutputEngineStore::getOutputEngineStore().readFile(filename,ext,conf));
  } else {
      throw VnVExceptionBase("Engine Detection Failed because output file has no extension.");
  }
}

ReaderWrapper::ReaderWrapper(std::string filename,std::string reader, std::string config)
{
    //Reset the engine manager, using the provided reader and config.
    json conf = json::parse(config);
    rootNode.reset(VnV::OutputEngineStore::getOutputEngineStore().readFile(filename,reader,conf));
}

Nodes::IRootNode *ReaderWrapper::get() { return rootNode.get(); }

void VnVFinalize() {
    RunTime::instance().Finalize();
}

namespace {
/**
 * @brief stringsToChars Utility to convert a vec of strings to a vec of char*
 * @param vec
 * @return
 */
std::vector<char*> stringsToChars(std::vector<std::string> vec) {
    std::vector<char*> cstrings ;
    cstrings.reserve(vec.size());
    for (int i = 0; i < vec.size(); i++) {
        cstrings.push_back(const_cast<char*>(vec[i].c_str()));
    }
    return cstrings;
}
}

void VnVInit_Str(std::vector<std::string> args, std::string configStr) {
    json conf = json::parse(configStr);
    int argc = args.size();
    std::vector<char*> cstrings = stringsToChars(args);
    char** argv = &cstrings[0];
    RunTime::instance().InitFromJson("PYTHON_READER",&argc, &argv, conf, nullptr);

}

void VnVInit(std::vector<std::string> args, std::string config) {
    int argc = args.size();
    std::vector<char*> cstrings = stringsToChars(args);
    char** argv = &cstrings[0];
    RunTime::instance().InitFromFile("PYTHON_READER",&argc, &argv, config, nullptr);
}

}
}
