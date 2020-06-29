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

class JsonParserImpl {
public:

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
  n->templ = getTemplate(n->package,n->name, "Test");
  n->children = genArrayNode(j["children"], "children",
                             j["id"].get<std::string>() + "-children");
  n->resultsMap = genMapNode(j["results"], "results",
                             j["id"].get<std::string>() + "-results");
  return n;
}

void appendTestNode(TestNode* test, const json& j) {
  for (auto it : j["children"]) {
    if (it["node"].get<std::string>().substr(0,4) == "Data" ) {
      test->data->add(nodeDispatcher(it));
    } else {
      test->children->add(nodeDispatcher(it));
    }
  }
}

std::shared_ptr<DataTypeNode> genDataTypeNode(const json& j) {
  auto n = mks<DataTypeNode>(j);
  n->children = std::make_shared<MapNode>();
  n->children->id = j["id"].get<std::string>() + "-children";
  n->children->name = "children";
  n->dataTypeName = j["dtype"];
  for (auto it : j["children"] ) {
     n->children->add(it["name"],nodeDispatcher(it));
  }
  return n;

}

std::shared_ptr<TestNode> genTestNode(const json& j) {
  auto n = mks<TestNode>(j);
  n->package = j["package"].get<std::string>();

  n->templ = getTemplate(n->package,n->name, "Test");
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
  n->templ = getTemplate(n->package,n->name, "InjectionPoint");
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
  else if (node == "DataNode")
    return genDataTypeNode(j);
  else if (node == "InjectionPoint")
    return genInjectionPointNode(j);
  else
    throw VnVExceptionBase("Unknown Node type");
}

json& mainJson;
JsonParserImpl(json &jj) : mainJson(jj){
}

std::string getTemplate(std::string pname, std::string key, std::string type) {
  if ( type  == "UnitTest" ) {
     return mainJson["spec"]["UnitTests"][pname+":"+key]["docs"];
    } else if( type == "Test") {
     return mainJson["spec"]["Tests"][pname+":"+key]["docs"];
    } else if (type == "InjectionPoint") {
      return mainJson["spec"]["InjectionPoints"][pname + ":"+key]["docs"];
    }
    return "";
}


RootNode* genRootNode() {
  RootNode* root = new RootNode();
  root->infoNode = genInfoNode(mainJson["info"]);

  root->children = std::make_shared<ArrayNode>();
  root->children->id = "-children";
  root->children->name = "children";

  root->unitTests = std::make_shared<ArrayNode>();
  root->unitTests->id = "-unitTests";
  root->children->name = "unitTests";

  for (auto it : mainJson["children"]) {
    if (it["node"].get<std::string>() == "unitTest") {
      root->unitTests->add(genUnitTestNode(it));
    } else {
      root->children->add(nodeDispatcher(it));
    }
  }
  return root;
}

};

}  // namespace

IRootNode* parse(std::string filename) {

  std::ifstream ifs(filename);
  json j = json::parse(ifs);
  JsonParserImpl impl(j);
  return impl.genRootNode();
}

}  // namespace JsonReader
}  // namespace Engines
}  // namespace PACKAGENAME
}  // namespace VnV
