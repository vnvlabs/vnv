#include "interfaces/nodes/NodesImpl.h"

#include <iostream>
#include <random>

namespace VnV {
namespace Nodes {

#define X(x, y)                                                  \
  x##Node::x##Node() : I##x##Node() {}                           \
  x##Node::~x##Node(){};                                         \
  const std::vector<std::size_t>& x##Node::getShape() { return shape; } \
  y x##Node::getValue(const std::vector<std::size_t>& rshape) {  \
    if (shape.size() == 0) {                                     \
      return value[0];                                           \
    }                                                            \
    int index = 0;                                               \
    for (int i = 0; i < shape.size(); i++) {                     \
      index += rshape[i] * shape[i];                             \
    }                                                            \
    return getValue(index);                                      \
  }                                                              \
  y x##Node::getValue(const std::size_t ind) {            \
    return value[ind];                                           \
  }
DTYPES
#undef X

TestNode::TestNode() : ITestNode() {}
std::string TestNode::getPackage() { return package; }
IArrayNode* TestNode::getData() { return data.get(); }
IArrayNode* TestNode::getChildren() { return children.get(); }

UnitTestNode::UnitTestNode() : IUnitTestNode() {}
std::string UnitTestNode::getPackage() { return package; }
IArrayNode* UnitTestNode::getChildren() { return children.get(); }
IMapNode* UnitTestNode::getResults() { return resultsMap.get(); }

IArrayNode* RootNode::getChildren() { return children.get(); }
IArrayNode* RootNode::getUnitTests() { return unitTests.get(); }
IInfoNode* RootNode::getInfoNode() { return infoNode.get(); }
ICommInfoNode* RootNode::getCommInfoNode() { return commInfo.get(); }

DataBase* RootNode::findById(long id) {
  if (id == getId()) return this;
  auto it = idMap.find(id);
  if (it != idMap.end())
    return it->second.get();
  else
    return nullptr;
}

ArrayNode::ArrayNode() : IArrayNode() {}
std::size_t ArrayNode::size() { return value.size(); }
IArrayNode* ArrayNode::add(std::shared_ptr<DataBase> data) {
  value.push_back(data);
  return this;
}
std::shared_ptr<DataBase> ArrayNode::getShared(std::size_t idx){
  return (idx < value.size()) ? (value[idx]) : nullptr;
}

LogNode::LogNode() : ILogNode() {}
std::string LogNode::getPackage() { return package; }
std::string LogNode::getLevel() { return level; }
std::string LogNode::getMessage() { return message; }
std::string LogNode::getStage() { return stage; }
std::string LogNode::def =
    R"(:vnv:`$.package` : :vnv:`$.stage`: :vnv:`$.message`)";

InjectionPointNode::InjectionPointNode() : IInjectionPointNode() {}
std::string InjectionPointNode::getPackage() { return package; }
IArrayNode* InjectionPointNode::getTests() { return tests.get(); }
IArrayNode* InjectionPointNode::getChildren() { return children.get(); }

InfoNode::InfoNode() : IInfoNode() {}
std::string InfoNode::getTitle() { return title; }
long InfoNode::getDate() { return date; }


CommInfoNode::CommInfoNode() : ICommInfoNode() {}
int CommInfoNode::getWorldSize() {return worldSize;};
std::string CommInfoNode::getCommMap() { return commMap.dump(); }


std::vector<std::string> MapNode::fetchkeys() {
  std::vector<std::string> v;
  for (auto it = value.begin(); it != value.end(); ++it) {
    v.push_back(it->first);
  }
  return v;
}
std::size_t MapNode::size() { return value.size(); }
bool MapNode::contains(std::string key) {
  return value.find(key) != value.end();
}
MapNode::MapNode() : IMapNode() {}

DataBase* MapNode::get(std::string key) {
  auto it = value.find(key);
  return (it == value.end()) ? nullptr : (it->second).get();
}

IMapNode* MapNode::add(std::string key, std::shared_ptr<DataBase> v) {
  value.insert(std::make_pair(key, v));
  return this;
}

DataNode::DataNode() : IDataNode() {}
long long DataNode::getDataTypeKey() { return key; }
IMapNode* DataNode::getChildren() { return children.get(); }

}  // namespace Nodes
}  // namespace VnV
