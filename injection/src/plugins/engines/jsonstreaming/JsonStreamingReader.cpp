#include <interfaces/ICommunicator.h>

#include <fstream>
#include <iostream>
#include <list>

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/nodes/NodesImpl.h"
#include "json-schema.hpp"
#include "plugins/engines/jsonstreaming/JsonStreamingEngineManager.h"

using nlohmann::json;

using namespace VnV::Nodes;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

/** Some utility functions */
namespace {

template <typename T> T getOrDefault(json j, std::string key, T def) {
  return j.contains(key) ? j[key].get<T>() : def;
}

void throwIfNotEqual(json j, std::string val, std::string key = JSD::node) {
  if (val.compare(getOrDefault<std::string>(j, key, "")) != 0) {
    throw VnVExceptionBase("Node type error");
  }
}

std::string getName(json j, std::string def = "") {
  return getOrDefault<std::string>(j, JSD::name, def);
}
}  // namespace

JsonIterator::JsonIterator() {};
JsonIterator::~JsonIterator() {};


class JsonParserVisitor {
 public:
  JsonStreamReader& jstream;
  std::shared_ptr<RootNode> rootInternal;
  long& idCounter;

  JsonParserVisitor(JsonStreamReader& jstream_, long& idStart)
      : jstream(jstream_), idCounter(idStart) {
    rootInternal.reset(new RootNode());  // For id tracking in a single loop.
    rootInternal->id = idCounter++;
    rootInternal->name = "Root";
    rootInternal->parent = {};
  }

  void process() {
    while (jstream.hasNext()) {
      JsonEntry e = jstream.next();
      visit(e.id, e.j);
    }
  }

  void childNodeDispatcher(std::shared_ptr<DataBase> child, const json& node) {
    
    
    auto parent = jstream.top();
    DataBase::DataType ptype = parent->getType();
    DataBase::DataType ctype = child->getType();

    if (ptype == DataBase::DataType::InjectionPoint) {
      auto p = std::dynamic_pointer_cast<InjectionPointNode>(parent);
      if (ctype == DataBase::DataType::Test) {
        p->tests->add(child);
        child->parent = {p->tests};
      } else {
        p->children->add(child);
        child->parent = {p->children};
      }
    } else if (ptype == DataBase::DataType::Test) {
      auto p1 = std::dynamic_pointer_cast<TestNode>(parent);
      if (ctype == DataBase::DataType::Log) {
        p1->children->add(child);
        child->parent = {p1->children};
      } else {
        p1->data->add(child);
        child->parent = {p1->data};
      }
    } else if (ptype == DataBase::DataType::UnitTest) {
      auto p = std::dynamic_pointer_cast<UnitTestNode>(parent);
      p->children->add(child);
      child->parent = {p->children};
    } else if (ptype == DataBase::DataType::Data) {
      auto p = std::dynamic_pointer_cast<DataNode>(parent);
      p->children->add(child->getName(), child);
      child->parent = {p->children};
    } else if (ptype == DataBase::DataType::Shape) {
      auto p = std::dynamic_pointer_cast<ShapeNode>(parent);
      p->value.push_back(child);
      child->parent = {p};
    } else {
      throw VnVExceptionBase("Unsupported Parent element type");
    }
  }

  void visit(long id, const json& j) {
    std::string node = j[JSD::node].get<std::string>();

    if (node == JSN::commInfo) {
      visitCommNode(j);
    } else if (node == JSN::dataTypeEnded) {
      visitDataNodeEnded(j);
    } else if (node == JSN::dataTypeStarted) {
      visitDataNodeStarted(j);
    } else if (node == JSN::info) {
      visitInfoNode(j);
    } else if (node == JSN::injectionPointEnded) {
      visitInjectionPointEndedNode(id, j);
    } else if (node == JSN::injectionPointStarted) {
      visitInjectionPointStartedNode(id, j);
    } else if (node == JSN::injectionPointIterEnded) {
      visitInjectionPointIterEndedNode(id, j);
    } else if (node == JSN::injectionPointIterStarted) {
      visitInjectionPointIterStartedNode(id, j);
    } else if (node == JSN::log) {
      visitLogNode(id, j);
    } else if (node == JSN::testFinished) {
      visitTestNodeEnded(j);
    } else if (node == JSN::testStarted) {
      visitTestNodeStarted(j);
    } else if (node == JSN::unitTestFinished) {
      visitUnitTestEndedNode(j);
    } else if (node == JSN::unitTestStarted) {
      visitUnitTestStartedNode(j);
    } else if (node == JSN::shape) {
      std::string type = j[JSD::dtype].get<std::string>();
      if (type == "std::string") {
        visitShapeNode<StringNode, std::string>(j);
      } else if (type == "bool") {
        visitShapeNode<BoolNode, bool>(j);
      } else if (type == "long long") {
        visitShapeNode<LongNode, long>(j);
      } else if (type == "double") {
        visitShapeNode<DoubleNode, double>(j);
      } else if (type == "json") {
        visitJsonNode(j);
      } else if (type == "ga") {
        visitGlobalArrayNode(j);
      }
    } else {
      throw VnVExceptionBase("Unrecognized Node Type");
    }
  }

  template <typename T>
  std::shared_ptr<T> mks(const json& j, std::string noName = "") {
    std::shared_ptr<T> base_ptr(new T());

    base_ptr->name = getName(j, noName);
    base_ptr->id = idCounter++;

    if (j.contains(JSD::meta)) {
      for (auto it : j[JSD::meta].items()) {
        base_ptr->getMetaData().add(it.key(), it.value());
      }
    }

    rootInternal->add(base_ptr);
    return base_ptr;
  }

  // Generate a data node (name, value,
  template <typename T, typename V>
  std::shared_ptr<T> visitShapeNode(const json& j) {
    throwIfNotEqual(j, JSN::shape);

    std::shared_ptr<T> n = mks<T>(j);

    std::size_t shapeSize = j[JSD::shape].size();

    // Set the shape object.
    n->shape.reserve(shapeSize);
    for (auto it : j[JSD::shape].items()) {
      n->shape.push_back(it.value().get<std::size_t>());
    }

    // Set the scalar value.
    if (shapeSize == 0) {
      n->value.reserve(1);
      n->value.push_back(j[JSD::value].get<V>());
    } else {
      n->value.reserve(shapeSize);
      for (auto it : j[JSD::value].items()) {
        n->value.push_back(it.value().get<V>());
      }
    }

    childNodeDispatcher(n, j);
    return n;
  }

  std::shared_ptr<JsonNode> visitJsonNode(const json& j) {
    auto n = mks<JsonNode>(j);

    throwIfNotEqual(j, JSN::shape);
    throwIfNotEqual(j, "json", JSD::dtype);

    std::size_t shapeSize = j[JSD::shape].size();
    n->shape.reserve(shapeSize);
    for (auto it : j[JSD::shape].items()) {
      n->shape.push_back(it.value().get<std::size_t>());
    }
    if (shapeSize == 0) {
      n->value.push_back(j[JSD::value].dump());
    } else {
      n->value.reserve(j[JSD::value].size());
      for (auto it : j[JSD::value].items()) {
        n->value.push_back(it.value().dump());
      }
    }
    childNodeDispatcher(n, j);
    return n;
  }

  std::shared_ptr<DataBase> visitGlobalArrayNode(const json& j) {
    throwIfNotEqual(j, JSN::shape);
    throwIfNotEqual(j, "ga", JSD::dtype);

    auto n = mks<ShapeNode>(j);

    // Load the shape.
    for (auto it : j[JSD::shape].items()) {
      n->shape.push_back(it.value().get<std::size_t>());
    }

    long long key = j[JSD::key].get<long long>();

    int count = 0;
    for (auto& it : j["children"].items()) {
      visit(count++, it.value());
    }
    childNodeDispatcher(n, j);
    return n;
  }

  std::shared_ptr<InfoNode> visitInfoNode(const json& j) {
    throwIfNotEqual(j, JSN::info);
    auto n = mks<InfoNode>(j);
    n->title = j["title"].get<std::string>();
    n->date = j["date"].get<long>();
    rootInternal->spec.reset(new VnVSpec(j[JSD::spec]));
    rootInternal->commInfo.reset(new CommInfoNode());
    rootInternal->commInfo->worldSize = j["worldSize"];

    n->parent = {rootInternal};
    rootInternal->infoNode = n;

    return n;
  }

  void visitCommNode(const json& j) {
    throwIfNotEqual(j, JSN::commInfo);

    std::set<int> s;
    for (auto& it : j[JSD::commList].items()) {
      s.insert(it.value().get<int>());
    }
    rootInternal->commInfo->commMap.add(j[JSD::comm].get<long>(), s);
  }

  std::shared_ptr<LogNode> visitLogNode(long id, const json& j) {
    auto n = mks<LogNode>(j, "log");

    n->package = j[JSD::package].get<std::string>();
    n->level = j[JSD::level].get<std::string>();
    n->message = j[JSD::message].get<std::string>();
    n->stage = std::to_string(j[JSD::stage].get<int>());
    n->comm = std::to_string(j[JSD::comm].get<long>());
    n->time = j[JSD::time].get<double>();
    n->identity = j[JSD::id].get<long>();
    childNodeDispatcher(n, j);

    return n;
  }

  std::shared_ptr<UnitTestNode> visitUnitTestStartedNode(const json& j) {
    throwIfNotEqual(j, JSN::unitTestStarted);

    auto n = mks<UnitTestNode>(j);
    n->package = j[JSD::package];

    json templs = rootInternal->spec->unitTest(n->package, n->name);
    n->templ = templs["docs"];
    n->resultsMap = mks<Nodes::UnitTestResultsNode>(json::object(), "results");
    n->resultsMap->parent = {n};

    for (auto it : templs["tests"].items()) {
      n->testTemplate[it.key()] = it.value().get<std::string>();
    }

    jstream.push(n);

    n->parent = {rootInternal->unitTests};
    rootInternal->unitTests->add(n);
    return n;
  }

  std::shared_ptr<UnitTestNode> visitUnitTestEndedNode(const json& j) {
    throwIfNotEqual(j, JSN::unitTestFinished);
    auto n = jstream.pop();
    if (!(n->getType() == DataBase::DataType::UnitTest)) {
      throw VnVExceptionBase("Bad Heirarchy");
    }
    std::shared_ptr<UnitTestNode> nn =
        std::dynamic_pointer_cast<UnitTestNode>(n);
    for (auto& it : j[JSD::results].items()) {
      auto r = mks<UnitTestResultNode>(it.value());
      r->parent = {nn->resultsMap};
      nn->resultsMap->m.insert(std::make_pair(r->name, r));
    }
    return nn;
  }

  std::shared_ptr<DataBase> visitDataNodeStarted(const json& j) {
    throwIfNotEqual(j, JSN::dataTypeStarted);

    auto n = mks<DataNode>(j);

    json jj = json::object();
    n->key = j[JSD::dtype].get<long long>();

    childNodeDispatcher(n, j);
    jstream.push(n);
    return n;
  }

  void visitDataNodeEnded(const json& j) {
    throwIfNotEqual(j, JSN::dataTypeEnded);
    auto n = jstream.pop();
    if (n->getType() != DataBase::DataType::Shape) {
      throw VnVExceptionBase("Bad Heirarchy");
    }
  }

  std::shared_ptr<TestNode> visitTestNodeStarted(const json& j) {
    throwIfNotEqual(j, JSN::testStarted);

    auto p = jstream.top();
    if (p->getType() != DataBase::DataType::InjectionPoint) {
      throw VnVExceptionBase("Bad Heirrarchy");
    }
    auto pp = std::dynamic_pointer_cast<InjectionPointNode>(p);
    if (pp->isIter) {
      long uid = j[JSD::testuid].get<long>();
      auto tests = pp->tests;
      for (auto it : tests->value) {
        auto t = std::dynamic_pointer_cast<TestNode>(it);
        if (uid == t->uid) {
          jstream.push(t);
          return t;
        }
      }
      throw VnVExceptionBase("Could not find matching test during iteration");
    }

    auto n = mks<TestNode>(j);
    n->uid = j[JSD::testuid].get<long>();
    n->package = j[JSD::package].get<std::string>();
    n->templ = (j[JSD::internal].get<bool>())
                   ? ""
                   : rootInternal->spec->test(n->package, n->name);

    childNodeDispatcher(n, j);
    jstream.push(n);
    return n;
  }

  void visitTestNodeEnded(const json& j) {
    throwIfNotEqual(j, JSN::testFinished);
    auto n = jstream.pop();
    if (n->getType() != DataBase::DataType::Test) {
      throw VnVExceptionBase("Bad Heirarchy");
    }

    std::shared_ptr<TestNode> nn = std::dynamic_pointer_cast<TestNode>(n);
    nn->result = j[JSD::result].get<bool>();
  }

  std::shared_ptr<InjectionPointNode> visitInjectionPointStartedNode(
      long id, const json& j) {
    auto n = mks<InjectionPointNode>(j);
    n->package = j[JSD::package].get<std::string>();
    n->templ = rootInternal->spec->injectionPoint(n->package, n->name);
    n->commId = j[JSD::comm].get<long>();
    n->startIndex = id;
    rootInternal->getChildren()->add(n);
    n->parent = {rootInternal->children};
    jstream.push(n);
    return n;
  }

  std::shared_ptr<InjectionPointNode> visitInjectionPointEndedNode(
      long id, const json& j) {
    auto n = jstream.pop();

    if (n->getType() != DataBase::DataType::InjectionPoint) {
      throw VnVExceptionBase("Bad Heirarchy");
    }

    auto nn = std::dynamic_pointer_cast<InjectionPointNode>(n);
    if (j[JSD::node].get<std::string>() == JSN::injectionPointEnded) {
      nn->endIndex = id;
    }
    return nn;
  }

  std::shared_ptr<InjectionPointNode> visitInjectionPointIterEndedNode(
    long id, const json& j) {
    auto n = jstream.top();

    if (n->getType() != DataBase::DataType::InjectionPoint) {
      throw VnVExceptionBase("Bad Heirarchy");
    }

    auto nn = std::dynamic_pointer_cast<InjectionPointNode>(n);
    nn->isIter = false;
    return nn;
  }

  std::shared_ptr<InjectionPointNode> visitInjectionPointIterStartedNode(
    long id, const json& j) {
    auto n = jstream.top();

    if (n->getType() != DataBase::DataType::InjectionPoint) {
      throw VnVExceptionBase("Bad Heirarchy");
    }

    auto nn = std::dynamic_pointer_cast<InjectionPointNode>(n);
    nn->isIter = true;

    return nn;
  }
};

class JsonFileStream : public VnV::VNVPACKAGENAME::Engines::JsonStreamReader {
  std::list<std::shared_ptr<JsonIterator>> instreams;
  std::list<std::shared_ptr<JsonIterator>>::iterator min;

 public:
  JsonFileStream(){};

  virtual void add(std::shared_ptr<JsonIterator> iter) {
    instreams.push_back(iter);
  }

  virtual bool isDone() override {
    for (auto it : instreams) {
      if (!it->isDone()) {
        return false;
      }
    }
    return true;
  }

  virtual bool hasNext() override {
    for (auto it : instreams) {
      if (!it->hasNext()) {
        return true;
      }
    }
    return false;
  }

  virtual JsonEntry next() override {
    // Find the oone with the lowest id.
    min = std::min_element(
        instreams.begin(), instreams.end(),
        [](const std::shared_ptr<JsonIterator>& x, const std::shared_ptr<JsonIterator>& y) {
          return (x->peekId() < y->peekId());
        });

    return (*min)->next();
  }

  void push(std::shared_ptr<DataBase> d) override { (*min)->push(d); }
  std::shared_ptr<DataBase> pop() override { return (*min)->pop(); }
  std::shared_ptr<DataBase> top() override { return (*min)->top(); }
};

// A static file iterator.
class JsonFileIterator : public JsonIterator {
  std::ifstream ifs;
  bool done;
  bool more;
  long currId;
  std::string currJson;
  std::string filename;

  void getLine() {
    std::string currline;
    if (std::getline(ifs, currline)) {
      std::size_t ind = currline.find_first_of(":");
      currId = std::atol(currline.substr(0, ind).c_str());
      currJson = currline.substr(ind + 1);
      more = true;
    } else {
      currId = std::numeric_limits<long>::max();
      currJson = "";
      more = false;
    }
  }

 public:
  JsonFileIterator(std::string filename_)
      : filename(filename_), ifs(filename_) {
    getLine();
  }

  bool hasNext() const override { return more; }

  bool isDone() const override { return more; }

  std::string getFileName() const { return filename; }

  JsonEntry next() override {
    JsonEntry ret(currId, json::parse(currJson));
    getLine();
    return std::move(ret);
  }

  long peekId() const override { return currId; }

  ~JsonFileIterator() override { ifs.close(); }

  bool operator==(const JsonFileIterator& lhs) {
    return filename.compare(lhs.filename) == 0;
  }

  std::stack<std::shared_ptr<DataBase>> stack;

  void push(std::shared_ptr<DataBase> d) { stack.push(d); }

  std::shared_ptr<DataBase> pop() {
    auto s = stack.top();
    stack.pop();
    return s;
  }

  std::shared_ptr<DataBase> top() { return stack.top(); }
};


std::shared_ptr<Nodes::IRootNode> parseJsonFileStream(long& idCounter,
                                                      std::string filestub) {
  JsonFileStream stream;

  std::string meta = DistUtils::join({filestub, ".meta"}, 0777, false);
  json j = VnV::JsonUtilities::load(meta);
  for (auto& it : j.items()) {
    long id = it.value().get<long>();
    std::string fname = DistUtils::join(
        {filestub, std::to_string(id) + JSD::extension}, 0777, false);
      stream.add(std::make_shared<JsonFileIterator>(fname));
  }

  JsonParserVisitor visitor(stream, idCounter);
  visitor.process();
  return visitor.rootInternal;
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
