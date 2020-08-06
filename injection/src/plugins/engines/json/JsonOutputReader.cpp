#include "plugins/engines/json/JsonOutputReader.h"
#include <iostream>
#include <random>

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace JsonReader {

DoubleNode::DoubleNode() : IDoubleNode() {}

double DoubleNode::getValue() { return value; }

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

DataBase* RootNode::findById(long id) {
  if (id == getId()) return this;
  auto it = idMap.find(id);
  if (it != idMap.end())
    return it->second.get();
  else
    return nullptr;
}

std::size_t ArrayNode::size() { return value.size(); }

IArrayNode* ArrayNode::add(std::shared_ptr<DataBase> data) {
  value.push_back(data);
  return this;
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

ArrayNode::ArrayNode() : IArrayNode() {}

DataBase* ArrayNode::get(std::size_t idx) {
  return (idx < value.size()) ? (value[idx]).get() : nullptr;
}

FloatNode::FloatNode() : IFloatNode() {}

float FloatNode::getValue() { return value; }

StringNode::StringNode() : IStringNode() {}

bool StringNode::isJson() { return jsonString; }

std::string StringNode::getValue() { return value; }

LongNode::LongNode() : ILongNode() {}

long LongNode::getValue() { return value; }

IntegerNode::IntegerNode() : IIntegerNode() {}

int IntegerNode::getValue() { return value; }

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

BoolNode::BoolNode() : IBoolNode() {}

bool BoolNode::getValue() { return value; }

MapNode::MapNode() : IMapNode() {}

DataBase* MapNode::get(std::string key) {
  auto it = value.find(key);
  return (it == value.end()) ? nullptr : (it->second).get();
}

IMapNode* MapNode::add(std::string key, std::shared_ptr<DataBase> v) {
  value.insert(std::make_pair(key, v));
  return this;
}

DataTypeNode::DataTypeNode() : IDataTypeNode() {}
std::string DataTypeNode::getDataTypeName() { return dataTypeName; }
IMapNode* DataTypeNode::getChildren() { return children.get(); }

}  // namespace JsonReader
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
