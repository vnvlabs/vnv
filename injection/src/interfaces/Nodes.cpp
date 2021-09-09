
#include "interfaces/Nodes.h"

#include <iostream>
#include <sstream>

#include "base/exceptions.h"
#include "base/stores/WalkerStore.h"
#include "json-schema.hpp"

namespace VnV {
namespace Nodes {

long DataBase::getId() {
  std::cout << "Getting Id " << id << std::endl;
  return id;
}

bool DataBase::check(DataType type) { return type == dataType; }

std::string DataBase::getName() {
  return (name.empty()) ? std::to_string(id) : name;
}

DataBase::DataType DataBase::getType() { return dataType; }

DataBase::DataBase(DataType type) : dataType(type) {}

std::string DataBase::getTypeStr() {
  switch (dataType) {
#define X(x, y)     \
  case DataType::x: \
    return #x;
    DTYPES
#undef X
#define X(x)        \
  case DataType::x: \
    return #x;
    STYPES
    RTYPES
#undef X
  default:
    return "Custom";
  }
  throw VnVExceptionBase("Impossible");
}

#define X(x, y)                                                     \
  I##x##Node::I##x##Node() : DataBase(DataBase::DataType::x) {}     \
  I##x##Node::~I##x##Node() {}                                      \
  I##x##Node* DataBase::getAs##x##Node() {                          \
    if (check(DataType::x)) return dynamic_cast<I##x##Node*>(this); \
    throw VnVExceptionBase("Invalid Cast to DataType::%s", #x);     \
  }
DTYPES
#undef X

#define X(x)                                                        \
  I##x##Node::I##x##Node() : DataBase(DataBase::DataType::x) {}     \
  I##x##Node::~I##x##Node() {}                                      \
  I##x##Node* DataBase::getAs##x##Node() {                          \
    if (check(DataType::x)) return dynamic_cast<I##x##Node*>(this); \
    throw VnVExceptionBase("Invalid Cast to DataType::%s", #x);     \
  }
STYPES
RTYPES
#undef X

DataBase::~DataBase() {}

void IArrayNode::iter(std::function<void(DataBase*)>& lambda) {
  for (std::size_t i = 0; i < size(); i++) {
    lambda(get(i));
  }
}

void IRootNode::addIDN(long id, long streamId, node_type type, long index,
                       long duration, std::string stage) {
  auto it = nodes.find(index);
  if (it == nodes.end()) {
    nodes[index] = std::list<IDN>();
    nodes[index].push_back(IDN(id, streamId, type, duration, stage));
  } else {
    (it->second).push_back(IDN(id, streamId, type, duration, stage));
  }

  // Tell any listeners that we recieved a new node.
  auto a = std::prev(nodes[index].end());
  for (auto& it : walkers) {
    it->callback(index, a);
  }
}

WalkerWrapper::WalkerWrapper(std::shared_ptr<IWalker> walker, IRootNode* root)
    : ptr(walker), rootNode(root) {
  node.reset(new WalkerNode());
}

WalkerNode* WalkerWrapper::next() {
  if (ptr->next(*node)) {
    return node.get();
  }
  node->item = rootNode;
  node->type = node_type::DONE;
  node->edges.clear();
  node->time = rootNode->getTotalDuration();
  return node.get();
}

WalkerWrapper IRootNode::getWalker(std::string package, std::string name,
                                   std::string config) {
  nlohmann::json j = nlohmann::json::parse(config);
  auto a = WalkerStore::instance().getWalker(package, name, this, j);
  if (a == nullptr) {
    throw VnVExceptionBase("No Walker with that name");
  }

  return WalkerWrapper(a, this);
}

}  // namespace Nodes
}  // namespace VnV
