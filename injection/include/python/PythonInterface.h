#ifndef IOUTPUTREADER_H
#define IOUTPUTREADER_H

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "base/exceptions.h"
#include "interfaces/nodes/Nodes.h"
//#include "json-schema.hpp"

namespace VnV {

namespace Python {

class ReaderWrapper {
 private:
  std::unique_ptr<Nodes::IRootNode> rootNode;

 public:
  ReaderWrapper(std::string filename);
  ReaderWrapper(std::string filename, std::string config);
  ReaderWrapper(std::string filename, std::string reader, std::string config);
  Nodes::IRootNode* get();
};

void VnVInit(std::vector<std::string> args, std::string configFilename);
void VnVInit_Str(std::vector<std::string> args, std::string configStr);
void VnVFinalize();

}  // namespace Python

}  // namespace VnV

#endif  // IOUTPUTREADER_H
