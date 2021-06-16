#include <interfaces/ICommunicator.h>

#include <fstream>
#include <iostream>

#include "base/exceptions.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/nodes/NodesImpl.h"
#include "json-schema.hpp"

using nlohmann::json;

using namespace VnV::Nodes;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace Json {

#define JN(j) j["name"].get<std::string>()

// Put all the parser functions in @non namespace. We don't need
// them outside this unit.
namespace {

class JsonParserImpl {
 public:
  template <typename T>
  std::shared_ptr<T> mks(const std::string& name, std::shared_ptr<DataBase> parent) {
    std::shared_ptr<T> base_ptr(new T());

    base_ptr->name = name;
    base_ptr->id = idCounter++;
    base_ptr->parent = {parent};

    rootInternal->add(base_ptr);

    return base_ptr;
  }

  template<typename T>
  void addMetaData(json j, std::shared_ptr<T> node) {
    if ( j.contains("meta") ) {
      for (auto it : j["meta"].items()) {
        node->getMetaData().add(it.key(),it.value());
      }
    }
  }

  // Generate a data node (name, value,
  template <typename T, typename V>
  std::shared_ptr<T> genDataNode(const json& j, std::shared_ptr<DataBase> parent) {
    std::shared_ptr<T> n = mks<T>(JN(j), parent);

    n->shape.reserve(j["shape"].size());
    for (auto it : j["shape"].items()) {
      n->shape.push_back(it.value().get<std::size_t>());
    }

    n->value.reserve(j["value"].size());
    for (auto it : j["value"].items()) {
       n->value.push_back(it.value().get<V>());
    }
    addMetaData(j,n);
    return n;
  }

  std::shared_ptr<JsonNode> genJsonNode(const json& j, std::shared_ptr<DataBase> parent) {
    auto n = mks<JsonNode>(JN(j), parent);

    n->shape.reserve(j["shape"].size());
    for (auto it : j["shape"].items()) {
      n->shape.push_back(it.value().get<std::size_t>());
    }

    n->value.reserve(j["value"].size());
    for (auto it : j["value"].items()) {
      n->value.push_back(it.value().dump());
    }
    addMetaData(j,n);
    return n;
  }


  std::shared_ptr<InfoNode> genInfoNode(const json& j, std::shared_ptr<DataBase> parent) {
    auto n = mks<InfoNode>(JN(j), parent);
    n->title = j["title"].get<std::string>();
    n->date = j["date"].get<long>();
    addMetaData(j,n);
    return n;
  }

  std::shared_ptr<LogNode> genLogNode(const json& j, std::shared_ptr<DataBase> parent) {
    auto n = mks<LogNode>(JN(j), parent);
    n->package = j["package"].get<std::string>();
    n->level = j["level"].get<std::string>();
    n->message = j["message"].get<std::string>();
    n->stage = std::to_string(j["stage"].get<int>());
    n->comm = std::to_string(j["comm"].get<long>());
    addMetaData(j,n);
    return n;
  }
  std::shared_ptr<ArrayNode> genArrayNode(const json& j, std::string name,
                                          std::shared_ptr<DataBase> parent) {
    auto n = mks<ArrayNode>(name, parent);
    for (auto& it : j) {
      n->add(nodeDispatcher(it, n));
    }
    return n;
  }

  std::shared_ptr<MapNode> genMapNode(const json& j, std::string name,
                                      std::shared_ptr<DataBase> parent) {
    auto n = mks<MapNode>(name, parent);
    for (auto& it : j.items()) {
      n->add(it.key(), nodeDispatcher(it.value(), n));
    }
    return n;
  }

  std::shared_ptr<UnitTestNode> genUnitTestNode(const json& j,
                                                std::shared_ptr<DataBase> parent) {
    auto n = mks<UnitTestNode>(JN(j), parent);
    n->package = j["package"];
    n->templ = getTemplate(n->package, n->name, "UnitTest");

    json& testTemplates =
        mainJson["spec"]["UnitTests"][n->package + ":" + n->name]["tests"];
    for (auto it : testTemplates.items()) {
      n->testTemplate[it.key()] = it.value().get<std::string>();
    }

    n->children = genArrayNode(j["children"], "children", n);
    n->resultsMap = genMapNode(j["results"], "results", n);
    addMetaData(j,n);
    return n;
  }

  void appendTestNode(TestNode* test, const json& j) {
    for (auto it : j["children"]) {
      if (it["node"].get<std::string>().substr(0, 4) == "Data") {
        test->data->add(nodeDispatcher(it, test->data));
      } else {
        test->children->add(nodeDispatcher(it, test->children));
      }
    }

  }

  std::shared_ptr<DataNode> genDataNode(const json& j, std::shared_ptr<DataBase> parent) {
    auto n = mks<DataNode>(j, parent);

    n->children = mks<MapNode>("children", n);
    n->key = j["dtype"].get<long long>();
    for (auto it : j["children"]) {
      n->children->add(it["name"], nodeDispatcher(it, n->children));
    }
    addMetaData(j,n);
    return n;
  }


  std::shared_ptr<DataBase> genShapeNode(const json& j, std::shared_ptr<DataBase> parent) {
     auto n = mks<ShapeNode>(j,parent);


     // Load the shape.
     for (auto it : j["shape"].items()) {
        n->shape.push_back(it.value().get<std::size_t>());
     }

     long long key = j["key"].get<long long>();

     // Children is a vector of maps defing the datatype (local)
     for (auto it : j["children"]) {

       // it is a map -> { "name" : { "shape" : {} , "value" : {} , "type" : {} } }
       auto dn = mks<DataNode>("props", n);
       auto dm = mks<MapNode>("map", dn);

       dn->key = key;
       dn->local = true;

       // Add dn as a child.
       n->value.push_back(dn);

       for (auto itt : it.items()) {
         Communication::SupportedDataType s = static_cast<Communication::SupportedDataType>( itt.value()["type"].get<int>());
         switch (s) {
         case Communication::SupportedDataType::DOUBLE:
           dm->add(itt.key(), genDataNode<DoubleNode, double>(itt.value(), n));
           break;
         case Communication::SupportedDataType::LONG:
           dm->add(itt.key(), genDataNode<LongNode, long>(itt.value(), n));
           break;
         case Communication::SupportedDataType::STRING:
           dm->add(itt.key(),
                   genDataNode<StringNode, std::string>(itt.value(), n));
           break;
         case Communication::SupportedDataType::JSON:
           dm->add(itt.key(), genJsonNode(itt.value(), n));
           break;
         }
       }
     }
     return n;
  }


  std::shared_ptr<TestNode> genTestNode(const json& j, std::shared_ptr<DataBase> parent) {
    auto n = mks<TestNode>(JN(j), parent);
    n->package = j["package"].get<std::string>();

    n->templ = (j["internal"].get<bool>())
                   ? ""
                   : getTemplate(n->package, n->name, "Test");
    // Pass all child type nodes to children, and all data nodes to data.
    n->children = mks<ArrayNode>("children", n);
    n->data = mks<ArrayNode>("data", n);
    addMetaData(j,n);
    appendTestNode(n.get(), j);
    return n;
  }

  std::shared_ptr<InjectionPointNode> genInjectionPointNode(const json& j,
                                                            std::shared_ptr<DataBase> parent) {
    auto n = mks<InjectionPointNode>(JN(j), parent);
    n->package = j["package"].get<std::string>();
    n->templ = getTemplate(n->package, n->name, "InjectionPoint");
    n->commId =j["comm"].get<long>();
    n->children = mks<ArrayNode>("children", n);

    n->tests = mks<ArrayNode>("tests", n);
    addMetaData(j,n);
    std::map<std::string, std::shared_ptr<TestNode>> testMap;

    for (auto child : j["children"]) {
      if (child["node"] == "InjectionPointStage") {
        // This is a stage node. So, this is mine. I need to
        // update my tests using it.
        for (auto sub : child["children"]) {
          if (sub["node"] == "Test") {
            std::string name = sub["name"].get<std::string>();
            std::string pname = sub["package"].get<std::string>();
            std::string testId = pname + ":" + name;
            auto it = testMap.find(testId);
            if (it != testMap.end()) {
              appendTestNode(it->second.get(), sub);
            } else {
              std::shared_ptr<TestNode> test = genTestNode(sub, n->tests);

              if (sub["internal"].get<bool>()) {
                n->internal = test;
              } else {
                n->tests->add(test);
              }
              testMap[testId] = test;
            }
          } else {
            n->children->add(nodeDispatcher(sub, n->children));
          }
        }
      } else {
        // This is something else (maybe a doc, log, injection point?)
        n->children->add(nodeDispatcher(child, n->children));
      }
    }
    return n;
  }



  std::shared_ptr<DataBase> nodeDispatcher(const json& j, std::shared_ptr<DataBase> parent) {
    std::string node = j["node"].get<std::string>();
    if (node == "Data") {
      std::string type = j["type"].get<std::string>();
      if (type == "double")
        return genDataNode<DoubleNode, double>(j, parent);
      else if (type == "float")
        return genDataNode<FloatNode, float>(j, parent);
      else if (type == "std::string")
        return genDataNode<StringNode, std::string>(j, parent);
      else if (type == "long long")
        return genDataNode<LongNode, long>(j, parent);
      else if (type == "bool")
        return genDataNode<BoolNode, bool>(j, parent);
      else if (type == "int")
        return genDataNode<IntegerNode, int>(j, parent);
      else if (type == "json")
        return genJsonNode(j, parent);
      else if (type == "shape") {
         return genShapeNode(j,parent);
      }
      else
        std::cout << j.dump() << std::endl;
        throw VnVExceptionBase("Unknown Data node type");
    } else if (node == "Log")
      return genLogNode(j, parent);
    else if (node == "Info")
      return genInfoNode(j, parent);
    else if (node == "Test")
      return genTestNode(j, parent);
    else if (node == "UnitTest")
      return genUnitTestNode(j, parent);
    else if (node == "DataNode")
      return genDataNode(j, parent);
    else if (node == "InjectionPoint")
      return genInjectionPointNode(j, parent);
    else
      std::cout << j.dump() << std::endl;
      throw VnVExceptionBase("Unknown Node type");
  }

  json& mainJson;
  std::shared_ptr<RootNode> rootInternal;
  long& idCounter;
  JsonParserImpl(json& jj, long& idStart) : mainJson(jj), idCounter(idStart) {}

  std::string getTemplate(std::string pname, std::string key,
                          std::string type) {
    try {
      if (type == "UnitTest") {
        return mainJson["spec"]["UnitTests"][pname + ":" + key]["docs"]
            .get<std::string>();
      } else if (type == "Test") {
        return mainJson["spec"]["Tests"][pname + ":" + key]["docs"]
            .get<std::string>();
      } else if (type == "InjectionPoint") {
        return mainJson["spec"]["InjectionPoints"][pname + ":" + key]["docs"]
            .get<std::string>();
      } else if (type == "DataType") {
        return mainJson["spec"]["DataTypes"][key]["docs"].get<std::string>();
      } else if (type == "Introduction" || type == "Conclusion") {
        return mainJson["spec"][type]["docs"].get<std::string>();
      } else {
      }
    } catch (...) {
      // It doesn't exist because of an error
    }
    return "";
  }

  int getWorldSize() {
    return mainJson["commMap"]["worldSize"].get<int>();
  }

  std::string getCommMap() {
    return mainJson["commMap"]["map"].dump(0);
  }

  std::shared_ptr<RootNode> genRootNode() {
    rootInternal.reset(new RootNode());  // For id tracking in a single loop.

    rootInternal->id = idCounter++;
    rootInternal->name = "Root";
    rootInternal->parent = {};

    auto a = mks<CommInfoNode>("commInfo", rootInternal);
    a->commMap = getCommMap();
    a->worldSize = getWorldSize();
    rootInternal->commInfo = a;

    rootInternal->intro = getTemplate("", "", "Introduction");
    rootInternal->concl = getTemplate("", "", "Conclusion");
    rootInternal->infoNode = genInfoNode(mainJson["info"], rootInternal);

    rootInternal->children = mks<ArrayNode>("children", rootInternal);
    rootInternal->unitTests = mks<ArrayNode>("unitTests", rootInternal);


    for (auto it : mainJson["children"]) {
      if (it["node"].get<std::string>() == "UnitTest") {
        rootInternal->unitTests->add(genUnitTestNode(it, rootInternal->unitTests));
      } else {
        rootInternal->children->add(nodeDispatcher(it, rootInternal->children));
      }
    }

    return rootInternal;
  }
};

}  // namespace

IRootNode* parse(std::string filename, long& idCounter) {
  std::ifstream ifs(filename);
  json j = json::parse(ifs);
  JsonParserImpl impl(j, idCounter);
  return new SharedRootNodeWrapper(impl.genRootNode());
}

}  // namespace Json
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
