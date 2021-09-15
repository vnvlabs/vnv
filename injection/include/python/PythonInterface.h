#ifndef IOUTPUTREADER_H
#define IOUTPUTREADER_H

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "base/exceptions.h"
#include "interfaces/Nodes.h"
//#include "json-schema.hpp"

namespace VnV {

namespace Python {

class ReaderWrapper {
 private:
  std::shared_ptr<Nodes::IRootNode> rootNode;
  long lowerId, upperId;

 public:
  ReaderWrapper(std::string filename);
  ReaderWrapper(std::string filename, std::string config);
  ReaderWrapper(std::string filename, std::string reader, std::string config);

  Nodes::IRootNode* get();

  long getLowerId() { return lowerId; }
  long getUpperId() { return upperId; }
};

bool VnVInit(std::vector<std::string> args, std::string configFilename);
bool VnVInit_Str(std::vector<std::string> args, std::string configStr);
bool VnVIsInitialized();
void VnVFinalize();
void VnVLoadPlugin(std::string name, std::string filepath);

std::string VnVDumpReaders();

}  // namespace Python

}  // namespace VnV

#endif  // IOUTPUTREADER_H
