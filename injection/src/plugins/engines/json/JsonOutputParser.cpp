#include "plugins/engines/json/JsonOutputReader.h"
#include <fstream>
#include <iostream>

#include "base/exceptions.h"
#include "json-schema.hpp"
using nlohmann::json;

namespace VnV {
namespace PACKAGENAME {
namespace Engines {
namespace JsonReader {

// Put all the parser functions in @non namespace. We don't need
// them outside this unit.
namespace {

std::shared_ptr<DataBase> nodeDispatcher(const json& j);

template <typename T> std::shared_ptr<T> mks(const json& json) {
  std::shared_ptr<T> base_ptr(new T());
  base_ptr->name = json["name"].get<std::string>();
  base_ptr->id = json["id"].get<std::string>();
  return base_ptr;
}
// Generate a data node (name, value,
template <typename T, typename V>
std::shared_ptr<T> genDataNode(const json& j) {
  std::shared_ptr<T> n = mks<T>(j);
  n->value = j["value"].get<V>();
  return n;
}

std::shared_ptr<StringNode> genJsonNode(const json& j) {
  std::shared_ptr<StringNode> n = mks<StringNode>(j);
  n->value = j["value"].dump(3);
  n->jsonString = true;
  return n;
}

std::shared_ptr<InfoNode> genInfoNode(const json& j) {
  auto n = mks<InfoNode>(j);
  n->title = j["title"].get<std::string>();
  n->date = j["date"].get<long>();
  return n;
}

std::shared_ptr<LogNode> genLogNode(const json& j) {
  auto n = mks<LogNode>(j);
  n->package = j["package"].get<std::string>();
  n->level = j["level"].get<std::string>();
  n->message = j["message"].get<std::string>();
  n->stage = j["stage"].get<int>();
  return n;
}
std::shared_ptr<ArrayNode> genArrayNode(const json& j, std::string name,
                                        std::string id) {
  auto n = std::make_shared<ArrayNode>();
  n->name = name;
  n->id = id;
  for (auto& it : j) {
    n->add(nodeDispatcher(it));
  }
  return n;
}

std::shared_ptr<MapNode> genMapNode(const json& j, std::string name,
                                    std::string id) {
  auto n = std::make_shared<MapNode>();
  n->name = name;
  n->id = id;
  for (auto& it : j.items()) {
    n->add(it.key(), nodeDispatcher(it.value()));
  }
  return n;
}

std::shared_ptr<UnitTestNode> genUnitTestNode(const json& j) {
  auto n = mks<UnitTestNode>(j);
  n->package = j["package"];
  n->children = genArrayNode(j["children"], "children",
                             j["id"].get<std::string>() + "-children");
  n->resultsMap = genMapNode(j["results"], "results",
                             j["id"].get<std::string>() + "-results");
  return n;
}

void appendTestNode(TestNode* test, const json& j) {
  for (auto it : j["children"]) {
    if (it["node"] == "Data") {
      test->data->add(nodeDispatcher(it));
    } else {
      test->children->add(nodeDispatcher(it));
    }
  }
}

std::shared_ptr<TestNode> genTestNode(const json& j) {
  auto n = mks<TestNode>(j);
  n->package = j["package"].get<std::string>();

  // Pass all child type nodes to children, and all data nodes to data.
  n->children = std::make_shared<ArrayNode>();
  n->children->id = j["id"].get<std::string>() + "-children";
  n->children->name = "children";

  n->data = std::make_shared<ArrayNode>();
  n->data->id = j["id"].get<std::string>() + "-data";
  n->data->name = "data";
  appendTestNode(n.get(), j);
  return n;
}

std::shared_ptr<InjectionPointNode> genInjectionPointNode(const json& j) {
  auto n = mks<InjectionPointNode>(j);
  n->package = j["package"].get<std::string>();

  n->children = std::make_shared<ArrayNode>();
  n->children->id = j["id"].get<std::string>() + "-children";
  n->children->name = "children";

  n->tests = std::make_shared<ArrayNode>();
  n->tests->id = j["id"].get<std::string>() + "-tests";
  n->tests->name = "tests";

  std::map<std::string, int> testMap;

  for (auto child : j["children"]) {
    if (child["node"] == "InjectionPointStage") {
      // This is a stage node. So, this is mine. I need to
      // update my tests using it.
      for (auto sub : child["children"]) {
        if (sub["node"] == "Test") {
          auto it = testMap.find(sub["id"].get<std::string>());
          if (it != testMap.end()) {
            TestNode* test = (TestNode*)n->tests->get(it->second);
            appendTestNode(test, sub);
          } else {
            std::shared_ptr<TestNode> test = genTestNode(sub);
            n->tests->add(test);
            testMap[test->getId()] = n->tests->size() - 1;
          }
        } else {
          n->children->add(nodeDispatcher(sub));
        }
      }

    } else {
      // This is something else (maybe a doc, log, injection point?)
      n->children->add(nodeDispatcher(child));
    }
  }
  return n;
}

std::shared_ptr<DataBase> nodeDispatcher(const json& j) {
  std::string node = j["node"].get<std::string>();

  if (node == "Data") {
    std::string type = j["type"].get<std::string>();
    if (type == "double")
      return genDataNode<DoubleNode, double>(j);
    else if (type == "float")
      return genDataNode<FloatNode, float>(j);
    else if (type == "std::string")
      return genDataNode<StringNode, std::string>(j);
    else if (type == "long")
      return genDataNode<LongNode, long>(j);
    else if (type == "bool")
      return genDataNode<BoolNode, bool>(j);
    else if (type == "int")
      return genDataNode<IntegerNode, int>(j);
    else if (type == "json")
      return genJsonNode(j);
    else
      throw VnVExceptionBase("Unknown Data node type");
  } else if (node == "Log")
    return genLogNode(j);
  else if (node == "Info")
    return genInfoNode(j);
  else if (node == "Test")
    return genTestNode(j);
  else if (node == "UnitTest")
    return genUnitTestNode(j);
  else if (node == "InjectionPoint")
    return genInjectionPointNode(j);
  else
    throw VnVExceptionBase("Unknown Node type");
}

RootNode* genRootNode(const json& j) {
  RootNode* root = new RootNode();
  root->infoNode = genInfoNode(j["info"]);

  root->children = std::make_shared<ArrayNode>();
  root->children->id = "-children";
  root->children->name = "children";

  root->unitTests = std::make_shared<ArrayNode>();
  root->unitTests->id = "-unitTests";
  root->children->name = "unitTests";

  for (auto it : j["children"]) {
    if (it["node"].get<std::string>() == "unitTest") {
      root->unitTests->add(genUnitTestNode(it));
    } else {
      root->children->add(nodeDispatcher(it));
    }
  }
  return root;
}

}  // namespace

IRootNode* parse(std::string filename) {
  std::ifstream ifs(filename);
  const json j = json::parse(ifs);
  return genRootNode(j);
}

}  // namespace JsonReader
}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
