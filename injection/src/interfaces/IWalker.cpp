#include "interfaces/IWalker.h"
#include "streaming/Nodes.h"

namespace VnV {

IWalker::IWalker(Nodes::IRootNode* root) : rootNode(root) {}

IWalker::~IWalker() {};
};