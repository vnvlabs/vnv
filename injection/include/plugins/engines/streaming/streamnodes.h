#ifndef STREAMNODES_TEMPLATES_H
#define STREAMNODES_TEMPLATES_H

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <list>
#include <stack>
#include <string>
#include <thread>

#include "interfaces/Nodes.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace Streaming {

using namespace Nodes;

class ArrayNode : public IArrayNode {
 public:
  std::string templ;
  std::vector<std::shared_ptr<DataBase>> value;

  ArrayNode(std::string name) : IArrayNode() { this->name = name; }

  virtual std::shared_ptr<DataBase> getShared(std::size_t idx) override {
    return (idx < value.size()) ? (value[idx]) : nullptr;
  }

  virtual std::size_t size() override { return value.size(); };

  virtual IArrayNode* add(std::shared_ptr<DataBase> data) override {
    value.push_back(data);
    return this;
  }

  std::string getValue() override { return templ; }
};

class MapNode : public IMapNode {
 public:
  std::string templ;
  std::map<std::string, std::shared_ptr<ArrayNode>> value;

  MapNode(std::string name) : IMapNode() { this->name = name; }

  virtual IArrayNode* get(std::string key) override {
    auto it = value.find(key);
    return (it == value.end()) ? nullptr : (it->second).get();
  }

  virtual bool contains(std::string key) override { return value.find(key) != value.end(); }

  virtual std::vector<std::string> fetchkeys() override {
    std::vector<std::string> v;
    for (auto it = value.begin(); it != value.end(); ++it) {
      v.push_back(it->first);
    }
    return v;
  }

  virtual std::string getValue() override { return templ; }

  virtual std::size_t size() override { return value.size(); };

  virtual ~MapNode(){};
};

#define X(x, y)                                                                        \
  class x##Node : public I##x##Node {                                                  \
   public:                                                                             \
    std::vector<std::size_t> shape;                                                    \
    std::vector<y> value;                                                              \
                                                                                       \
    x##Node(std::string name) : I##x##Node() { this->name = name; }                    \
                                                                                       \
    const std::vector<std::size_t>& getShape() override { return shape; }              \
                                                                                       \
    y getValueByShape(const std::vector<std::size_t>& rshape) override {               \
      if (shape.size() == 0) {                                                         \
        return value[0];                                                               \
      }                                                                                \
      if (rshape.size() != shape.size()) throw VnVExceptionBase("Invalid Shape");      \
      int index = 0;                                                                   \
      for (int i = 0; i < shape.size(); i++) {                                         \
        index += rshape[i] * shape[i];                                                 \
      }                                                                                \
      return getValueByIndex(index);                                                   \
    }                                                                                  \
                                                                                       \
    y getValueByIndex(const size_t ind) override { return value[ind]; }                \
                                                                                       \
    y getScalarValue() override {                                                      \
      if (shape.size() == 0)                                                           \
        return value[0];                                                               \
      else                                                                             \
        throw VnVExceptionBase("No shape provided to non scalar shape tensor object"); \
    }                                                                                  \
                                                                                       \
    int getNumElements() override { return value.size(); }                             \
    std::vector<y>& getRawVector() override { return value; }                          \
    virtual ~x##Node() {}                                                              \
  };  // namespace Engines
DTYPES
#undef X

class InfoNode : public IInfoNode {
 public:
  long date;
  std::string title, templ;
  std::shared_ptr<VnVProv> prov;

  InfoNode(std::string name) : IInfoNode() { this->name = name; }
  virtual std::string getTitle() override { return title; }
  virtual long getDate() override { return date; }
  virtual std::string getValue() override { return templ; }

  virtual VnVProv* getProv() override { return prov.get(); }
  virtual ~InfoNode() {}
};

class CommMap : public ICommMap {
  class Comm;
  typedef std::shared_ptr<Comm> Comm_ptr;

  class Comm {
    void getAllChildren(std::set<long>& data, std::map<long, Comm_ptr>& result) {
      for (auto& it : children) {
        if (data.find(it.first) == data.end()) {
          it.second->getAllChildren(data, result);
          data.insert(it.first);
          result.erase(it.first);
        }
      }
    }

   public:
    long id;
    std::set<long> procs;
    std::map<long, Comm_ptr> children;
    std::map<long, Comm_ptr> parents;

    Comm(std::set<long> procs_, long id_) : procs(procs_), id(id_) {}

    bool isRoot() { return parents.size() == 0; }

    std::map<long, Comm_ptr>& getChildren() { return children; }

    void getAllChildren(std::map<long, Comm_ptr>& data) {
      for (auto& it : children) {
        if (data.find(it.first) == data.end()) {
          it.second->getAllChildren(data);
          data.insert(it);
        }
      }
    }

    void getDirectChildren(bool strip, std::map<long, Comm_ptr>& data) {
      if (!strip) {
        getAllChildren(data);
      } else {
        // A direct child is any child that is not also a child of one of  my
        // children.
        std::set<long> grandChildren;
        for (auto& it : children) {
          // If we havnt found this as a grand child already
          if (grandChildren.find(it.first) == grandChildren.end()) {
            // add it to my list of all direct children.
            data.insert(it);
            // Now add all its children -- it will be removed if found as a
            // grand child.
            it.second->getAllChildren(grandChildren, data);
          }
        }
      }
    }

    std::map<long, Comm_ptr>& getParents() { return parents; }

    void getAllParents(std::map<long, Comm_ptr>& data) {
      for (auto& it : parents) {
        if (data.find(it.first) == data.end()) {
          data.insert(it);
          it.second->getAllParents(data);
        }
      }
    }

    std::map<long, Comm_ptr> commChain() {
      std::map<long, Comm_ptr> m;
      getAllParents(m);
      getAllChildren(m);
      return m;
    }

    nlohmann::json getParentChain() {
      nlohmann::json j = nlohmann::json::array();
      std::map<long, Comm_ptr> m;
      getAllParents(m);
      for (auto it : m) {
        j.push_back(it.first);
      }
      return j;
    }

    bool is_parent(long id) {
      std::map<long, Comm_ptr> m;
      getAllParents(m);
      return m.find(id) != m.end();
    }

    bool is_child(long id) {
      for (auto& it : children) {
        if (id == it.first) {
          return true;
        }
      }
      return false;
    }

    nlohmann::json getChildChain() {
      nlohmann::json j = nlohmann::json::array();
      std::map<long, Comm_ptr> m;
      getAllChildren(m);
      for (auto it : m) {
        j.push_back(it.first);
      }
      return j;
    }

    void toJson(bool strip, nlohmann::json& j, std::set<long>& done) {
      if (done.find(id) == done.end()) {
        done.insert(id);
        nlohmann::json node = nlohmann::json::object();
        node["id"] = id;
        node["group"] = procs.size();
        node["parents"] = getParentChain();
        node["children"] = getChildChain();
        j["nodes"].push_back(node);

        std::map<long, Comm_ptr> ch;
        getDirectChildren(strip, ch);

        for (auto& it : ch) {
          nlohmann::json link = nlohmann::json::object();
          link["target"] = it.first;
          link["source"] = id;
          link["value"] = it.second->procs.size();
          j["links"].push_back(link);

          it.second->toJson(strip, j, done);
        }
      }
    }
  };

  std::map<long, Comm_ptr> nodes;

  void addChild(Comm_ptr ptr1, Comm_ptr ptr2) {
    // A Comm is my child if I contain all of its processors.
    // That can only be true if it is smaller than me.
    int sizeDiff = ptr1->procs.size() - ptr2->procs.size();

    Comm_ptr parent, child;

    if (sizeDiff == 0) {
      return;  // Cant be child if same size. (assume unique)
    } else if (sizeDiff < 0) {
      parent = ptr2;
      child = ptr1;
    } else {
      child = ptr2;
      parent = ptr1;
    }

    auto child_it = child->procs.begin();
    auto parent_it = parent->procs.begin();

    while (child_it != child->procs.end() && parent_it != parent->procs.end()) {
      if (*parent_it == *child_it) {
        ++parent_it;
        ++child_it;  // Found this one. so move on to next for both.
      } else if (*parent_it < *child_it) {
        ++parent_it;  // parent is less -- this is ok, parent can have elements
                      // not in child.
      } else {
        return;  // parent is greater -- Not ok, this means child_it is not in
                 // parent --> not child.
      }
    }

    if (child_it == child->procs.end()) {
      child->parents.insert(std::make_pair(parent->id, parent));

      parent->children.insert(std::make_pair(child->id, child));

    } else {
      return;  // We ran out of parent elements to check so it wqasnt there.
    }
  }

 public:
  CommMap(std::string name) : ICommMap() {}

  void add(long id, std::set<long>& comms) {
    Comm_ptr p = std::make_shared<Comm>(comms, id);
    for (auto& it : nodes) {
      addChild(p, it.second);
    }
    nodes.insert(std::make_pair(p->id, p));
  }

  std::set<long> commChain(long comm) {
    std::set<long> r;
    auto n = nodes.find(comm);
    if (n != nodes.end()) {
      for (auto it : n->second->commChain()) {
        r.insert(it.first);
      }
    }
    return r;
  }

  bool commContainsProcessor(long commId, long proc) override {
    auto n = nodes.find(commId);
    if (n != nodes.end()) {
      return n->second->procs.find(proc) != n->second->procs.end();
    }
    return false;
  }

  bool commIsSelf(long commId, long proc) {
    auto n = nodes.find(commId);
    if (n != nodes.end()) {
      return (n->second->procs.size() == 1 && *n->second->procs.begin() == proc);
    }
    return false;
  }

  bool commsIntersect(long streamId, long comm) {
    auto child = nodes.find(streamId);
    auto parent = nodes.find(comm);
    if (child == nodes.end() && parent != nodes.end()) {
      auto child_it = child->second->procs.begin();
      auto parent_it = parent->second->procs.begin();

      while (child_it != child->second->procs.end() && parent_it != parent->second->procs.end()) {
        if (*parent_it < *child_it) {
          ++parent_it;
        } else if (*child_it < *parent_it) {
          ++child_it;
        } else {
          return true;
        }
      }
    }
    return false;
  }

  bool commContainsComm(long commId, long childId) override {
    if (commId == childId) return true;

    auto n = nodes.find(commId);
    auto c = nodes.find(childId);

    // parent is smaller than child.
    if (n->second->procs.size() < c->second->procs.size()) {
      return false;
    }
    return c->second->is_parent(commId);
  }

  bool commIsChild(long parentId, long childId) { return nodes.find(parentId)->second->is_child(childId); }

  nlohmann::json listComms() {
    json j = json::object();
    for (auto& it : nodes) {
      j[std::to_string(it.first)] = it.second->procs;
    }
    return j;
  }

  nlohmann::json toJson(bool strip) {
    nlohmann::json j = R"({"nodes":[],"links":[]})"_json;
    std::set<long> done;
    for (auto& it : nodes) {
      it.second->toJson(strip, j, done);
    }
    return j;
  }

  std::string toJsonStr(bool strip) override { return toJson(strip).dump(); }

  ~CommMap() {}
};

class CommInfoNode : public ICommInfoNode {
 public:
  std::shared_ptr<CommMap> commMap;
  int worldSize;
  json nodeMap;
  std::string version;

  CommInfoNode(std::string name) : ICommInfoNode(), commMap(new CommMap("Communication Map")) { this->name = name; }

  virtual int getWorldSize() override { return worldSize; }
  virtual ICommMap* getCommMap() override { return commMap.get(); }
  virtual std::string getNodeMap() override { return nodeMap.dump(); }
  virtual std::string getVersion() override { return version; }

  virtual ~CommInfoNode(){};
};

class TestNode : public ITestNode {
 public:
  long uid;

  std::string package, templ;
  std::shared_ptr<MapNode> data;
  std::shared_ptr<ArrayNode> logs;
  bool result;
  bool internal;

  TestNode(std::string name) : ITestNode(), data(new MapNode("Data")), logs(new ArrayNode("Logs")) {
    this->name = name;
  }
  virtual std::string getPackage() override { return package; }
  virtual IMapNode* getData() override { return data.get(); }
  virtual IArrayNode* getLogs() override { return logs.get(); }
  virtual std::string getValue() override { return templ; }
  virtual bool isInternal() override { return internal; }

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) override {
    data->id = idCounter++;
    rootNode->add(data);
    logs->id = idCounter++;
    rootNode->add(logs);
  }
};

class InjectionPointNode : public IInjectionPointNode {
 public:
  std::shared_ptr<ArrayNode> children, logs;
  std::shared_ptr<ArrayNode> tests;
  std::shared_ptr<TestNode> internal;
  std::string package;
  std::string templ;

  nlohmann::json sourceMap = json::object();

  void addToSourceMap(std::string stage, std::string function, int line) {
    if (!sourceMap.contains(stage)) {
      json j = json::array();
      j.push_back(function);
      j.push_back(line);
      sourceMap[stage] = j;
    }
  }

  long startIndex = -1;
  long endIndex = -1;
  long long commId;

  bool isIter = false;    // internal property to help with parsing.
  bool __isOpen = false;  // internal property to help with paresing

  InjectionPointNode(std::string name)
      : IInjectionPointNode(),
        tests(new ArrayNode("Tests")),
        children(new ArrayNode("Children")),
        logs(new ArrayNode("Logs")) {
    this->name = name;
  }
  virtual std::string getPackage() override { return package; }
  virtual IArrayNode* getTests() override { return tests.get(); }
  virtual ITestNode* getData() override { return internal.get(); }
  virtual std::string getComm() override { return std::to_string(commId); }
  virtual std::string getValue() override { return templ; }
  virtual ArrayNode* getLogs() override { return logs.get(); };
  virtual std::string getSourceMap() override { return sourceMap.dump(); }

  virtual long getStartIndex() override { return startIndex; }
  virtual long getEndIndex() override { return endIndex; }

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) override {
    children->id = idCounter++;
    rootNode->add(children);
    tests->id = idCounter++;
    rootNode->add(tests);
    logs->id = idCounter++;
    rootNode->add(logs);
  }

  virtual ~InjectionPointNode() {}
};

class LogNode : public ILogNode {
 public:
  std::string package, level, stage, message, templ, comm;
  int identity;

  LogNode(std::string name) : ILogNode() { this->name = name; }
  virtual std::string getPackage() override { return package; }
  virtual std::string getLevel() override { return level; }
  virtual std::string getMessage() override { return message; }
  virtual std::string getComm() override { return comm; }

  virtual std::string getValue() override { return message; }
  virtual std::string getStage() override { return stage; }
  virtual ~LogNode() {}
};

class DataNode : public IDataNode {
 public:
  bool local;
  long long key;
  std::shared_ptr<ArrayNode> logs;
  std::string package, templ;
  std::shared_ptr<MapNode> children;

  DataNode(std::string name) : children(new MapNode("Data")), logs(new ArrayNode("Logs")) { this->name = name; }

  virtual ArrayNode* getLogs() override { return logs.get(); };
  virtual bool getLocal() override { return local; }
  virtual long long getDataTypeKey() override { return key; }
  virtual IMapNode* getData() override { return children.get(); }
  virtual std::string getValue() override { return templ; }
  virtual ~DataNode() {}

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) override {
    children->id = idCounter++;
    rootNode->add(children);
    logs->id = idCounter++;
    rootNode->add(logs);
  }
};

class UnitTestResultNode : public IUnitTestResultNode {
 public:
  std::string desc;
  std::string templ;
  bool result;

  UnitTestResultNode(std::string name) : IUnitTestResultNode() { this->name = name; }
  virtual std::string getDescription() override { return desc; }
  virtual bool getResult() override { return result; }
  virtual std::string getTemplate() override { return templ; }
  virtual ~UnitTestResultNode() {}
};

class UnitTestResultsNode : public IUnitTestResultsNode {
 public:
  std::shared_ptr<MapNode> m;

  UnitTestResultsNode(std::string name) : IUnitTestResultsNode(), m(new MapNode("Results")) { this->name = name; }

  virtual IUnitTestResultNode* get(std::string key) {
    if (m->contains(key)) {
      return m->get(key)->getAsArrayNode()->get(0)->getAsUnitTestResultNode();
    }
    throw VnVExceptionBase("Key error");
  };

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) override {
    m->id = idCounter++;
    rootNode->add(m);
  }

  virtual bool contains(std::string key) { return m->contains(key); }

  virtual std::vector<std::string> fetchkeys() { return m->fetchkeys(); };

  virtual ~UnitTestResultsNode(){};
};

class UnitTestNode : public IUnitTestNode {
 public:
  std::string package, templ;
  std::shared_ptr<ArrayNode> logs;
  std::shared_ptr<MapNode> children;
  std::shared_ptr<UnitTestResultsNode> resultsMap;
  std::map<std::string, std::string> testTemplate;

  UnitTestNode(std::string name)
      : IUnitTestNode(),
        resultsMap(new UnitTestResultsNode("Results Node")),
        children(new MapNode("Data")),
        logs(new ArrayNode("Logs")) {
    this->name = name;
  }

  virtual std::string getPackage() override { return package; }
  virtual IMapNode* getData() override { return children.get(); }
  virtual std::string getValue() override { return templ; }
  virtual ArrayNode* getLogs() override { return logs.get(); };

  virtual std::string getTestTemplate(std::string name) override {
    auto it = testTemplate.find(name);
    return (it == testTemplate.end()) ? "" : it->second;
  }

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) override {
    children->id = idCounter++;
    rootNode->add(children);
    resultsMap->id = idCounter++;
    rootNode->add(resultsMap);
    logs->id = idCounter++;
    rootNode->add(logs);
  }

  virtual IUnitTestResultsNode* getResults() override { return resultsMap.get(); }
};

class VisitorLock {
 public:
  VisitorLock() {}
  virtual ~VisitorLock() {}
  virtual void lock() = 0;
  virtual void release() = 0;
};

class RootNode : public IRootNode {
 public:
  long lowerId, upperId;
  long duration = -1;
  std::atomic<bool> _processing = ATOMIC_VAR_INIT(true);

  std::shared_ptr<VnVSpec> spec;
  std::shared_ptr<ArrayNode> children;
  std::shared_ptr<ArrayNode> unitTests;
  std::shared_ptr<MapNode> actions;
  std::shared_ptr<MapNode> packages;
  std::shared_ptr<InfoNode> infoNode;
  std::shared_ptr<CommInfoNode> commInfo;

  VisitorLock* visitorLocking = nullptr;

  RootNode()
      : spec(new VnVSpec()),
        children(new ArrayNode("Children")),
        unitTests(new ArrayNode("Unit Tests")),
        packages(new MapNode("Packages")),
        actions(new MapNode("Actions")),
        infoNode(new InfoNode("Info")),
        commInfo(new CommInfoNode("Communication Info")) {}

  virtual IMapNode* getPackages() override { return packages.get(); }
  virtual IMapNode* getActions() override { return actions.get(); }
  virtual IArrayNode* getChildren() override { return children.get(); }
  virtual IArrayNode* getUnitTests() override { return unitTests.get(); }
  virtual IInfoNode* getInfoNode() override { return infoNode.get(); }
  virtual ICommInfoNode* getCommInfoNode() override { return commInfo.get(); }
  virtual long getTotalDuration() override { return duration; }

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) override {
    children->id = idCounter++;
    rootNode->add(children);
    unitTests->id = idCounter++;
    rootNode->add(unitTests);
    packages->id = idCounter++;
    rootNode->add(packages);
    actions->id = idCounter++;
    rootNode->add(actions);
    infoNode->id = idCounter++;
    rootNode->add(infoNode);
    commInfo->id = idCounter++;
    rootNode->add(commInfo);
  }

  virtual bool processing() const override { return _processing.load(); }

  void lock() override {
    if (visitorLocking != NULL) {
      visitorLocking->lock();
    }
  }
  void release() override {
    if (visitorLocking != NULL) {
      visitorLocking->release();
    }
  }

  virtual const VnVSpec& getVnVSpec() { return *spec; }
};
}  // namespace Streaming
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif