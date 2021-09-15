#ifndef ENGINE_CONSTANTS_HEADER
#define ENGINE_CONSTANTS_HEADER

#include <atomic>
#include <chrono>
#include <functional>
#include <list>
#include <stack>
#include <string>

#include "base/DistUtils.h"
#include "base/Runtime.h"
#include "interfaces/Nodes.h"

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace Streaming {

#define STREAM_IS_CLOSED -199

using namespace Nodes;

namespace JSD {

constexpr auto outputFile = "outputFile";

#define NTYPES   \
  X(id)          \
  X(name)        \
  X(package)     \
  X(value)       \
  X(shape)       \
  X(node)        \
  X(meta)        \
  X(comm)        \
  X(children)    \
  X(prov)        \
  X(duration)    \
  X(key)         \
  X(stage)       \
  X(message)     \
  X(internal)    \
  X(description) \
  X(result)      \
  X(stageId)     \
  X(mpiversion)  \
  X(input)       \
  X(reader)      \
  X(level)       \
  X(dtype)       \
  X(endid)       \
  X(line)        \
  X(filename)    \
  X(results)     \
  X(time)        \
  X(spec) X(commList) X(testuid) X(sdt) X(title) X(nodeMap) X(worldSize) X(date)
#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES

}  // namespace JSD

namespace JSN {
#define NTYPES                                                               \
  X(log)                                                                     \
  X(shape)                                                                   \
  X(dataTypeStarted)                                                         \
  X(dataTypeEnded)                                                           \
  X(injectionPointStarted)                                                   \
  X(injectionPointEnded)                                                     \
  X(injectionPointIterStarted)                                               \
  X(injectionPointIterEnded)                                                 \
  X(packageOptionsStarted)                                                   \
  X(packageOptionsFinished)                                                  \
  X(testStarted)                                                             \
  X(file)                                                                    \
  X(done) X(duration) X(testFinished) X(unitTestStarted) X(unitTestFinished) \
      X(commInfo) X(info)

#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES
}  // namespace JSN

namespace JST {
#define NTYPES X(Double) X(Long) X(String) X(Json) X(Bool) X(GlobalArray)
#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES
}  // namespace JST

class ArrayNode : public IArrayNode {
 public:
  std::string templ;
  std::vector<std::shared_ptr<DataBase>> value;

  ArrayNode() : IArrayNode() {}

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

  MapNode() : IMapNode() {}

  virtual IArrayNode* get(std::string key) override {
    auto it = value.find(key);
    return (it == value.end()) ? nullptr : (it->second).get();
  }

  virtual IMapNode* add(std::string key, std::shared_ptr<DataBase> v) override {
    auto it = value.find(key);
    if (it == value.end()) {
      auto a = std::make_shared<ArrayNode>();
      a->add(v);
      value.insert(std::make_pair(key, a));
    } else {
      it->second->add(v);
    }

    return this;
  }

  virtual bool contains(std::string key) override {
    return value.find(key) != value.end();
  }

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

#define X(x, y)                                                           \
  class x##Node : public I##x##Node {                                     \
   public:                                                                \
    std::vector<std::size_t> shape;                                       \
    std::vector<y> value;                                                 \
                                                                          \
    x##Node() : I##x##Node() {}                                           \
                                                                          \
    const std::vector<std::size_t>& getShape() override { return shape; } \
                                                                          \
    y getValueByShape(const std::vector<std::size_t>& rshape) override {  \
      if (shape.size() == 0) {                                            \
        return value[0];                                                  \
      }                                                                   \
      if (rshape.size() != shape.size())                                  \
        throw VnVExceptionBase("Invalid Shape");                          \
      int index = 0;                                                      \
      for (int i = 0; i < shape.size(); i++) {                            \
        index += rshape[i] * shape[i];                                    \
      }                                                                   \
      return getValueByIndex(index);                                      \
    }                                                                     \
                                                                          \
    y getValueByIndex(const size_t ind) override { return value[ind]; }   \
                                                                          \
    y getScalarValue() override {                                         \
      if (shape.size() == 0)                                              \
        return value[0];                                                  \
      else                                                                \
        throw VnVExceptionBase(                                           \
            "No shape provided to non scalar shape tensor object");       \
    }                                                                     \
                                                                          \
    int getNumElements() override { return value.size(); }                \
    std::vector<y>& getRawVector() override { return value; }             \
    virtual ~x##Node() {}                                                 \
  };  // namespace Engines
DTYPES
#undef X

class InfoNode : public IInfoNode {
 public:
  long date;
  std::string title, templ;
  std::shared_ptr<VnVProv> prov;

  InfoNode() : IInfoNode() {}
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
    void getAllChildren(std::set<long>& data,
                        std::map<long, Comm_ptr>& result) {
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
  CommMap() : ICommMap() {}

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
      return (n->second->procs.size() == 1 &&
              *n->second->procs.begin() == proc);
    }
    return false;
  }

  bool commsIntersect(long streamId, long comm) {
    auto child = nodes.find(streamId);
    auto parent = nodes.find(comm);
    if (child == nodes.end() && parent != nodes.end()) {
      auto child_it = child->second->procs.begin();
      auto parent_it = parent->second->procs.begin();

      while (child_it != child->second->procs.end() &&
             parent_it != parent->second->procs.end()) {
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

  bool commIsChild(long parentId, long childId) {
    return nodes.find(parentId)->second->is_child(childId);
  }

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

  CommInfoNode() : ICommInfoNode(), commMap(new CommMap()) {}

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

  TestNode() : ITestNode(), data(new MapNode()), logs(new ArrayNode()) {}
  virtual std::string getPackage() override { return package; }
  virtual IMapNode* getData() override { return data.get(); }
  virtual IArrayNode* getLogs() override { return logs.get(); }
  virtual std::string getValue() override { return templ; }
  virtual bool isInternal() override { return internal; }
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

  InjectionPointNode()
      : IInjectionPointNode(),
        tests(new ArrayNode()),
        children(new ArrayNode),
        logs(new ArrayNode()) {}
  virtual std::string getPackage() override { return package; }
  virtual IArrayNode* getTests() override { return tests.get(); }
  virtual ITestNode* getData() override { return internal.get(); }
  virtual std::string getComm() override { return std::to_string(commId); }
  virtual std::string getValue() override { return templ; }
  virtual ArrayNode* getLogs() override { return logs.get(); };
  virtual std::string getSourceMap() override { return sourceMap.dump(); }

  virtual long getStartIndex() override { return startIndex; }
  virtual long getEndIndex() override { return endIndex; }
  virtual ~InjectionPointNode() {}
};

class LogNode : public ILogNode {
 public:
  std::string package, level, stage, message, templ, comm;
  int identity;

  LogNode() : ILogNode() {}
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

  DataNode() : children(new MapNode()), logs(new ArrayNode()) {}

  virtual ArrayNode* getLogs() override { return logs.get(); };
  virtual bool getLocal() override { return local; }
  virtual long long getDataTypeKey() override { return key; }
  virtual IMapNode* getData() override { return children.get(); }
  virtual std::string getValue() override { return templ; }
  virtual ~DataNode() {}
};

class UnitTestResultNode : public IUnitTestResultNode {
 public:
  std::string desc;
  std::string templ;
  bool result;

  UnitTestResultNode() : IUnitTestResultNode() {}
  virtual std::string getDescription() override { return desc; }
  virtual bool getResult() override { return result; }
  virtual std::string getTemplate() override { return templ; }
  virtual ~UnitTestResultNode() {}
};

class UnitTestResultsNode : public IUnitTestResultsNode {
 public:
  std::shared_ptr<MapNode> m;

  UnitTestResultsNode() : IUnitTestResultsNode(), m(new MapNode()) {}

  virtual IUnitTestResultNode* get(std::string key) {
    if (m->contains(key)) {
      return m->get(key)->getAsUnitTestResultNode();
    }
    throw VnVExceptionBase("Key error");
  };

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

  UnitTestNode()
      : IUnitTestNode(),
        resultsMap(new UnitTestResultsNode()),
        children(new MapNode()),
        logs(new ArrayNode()) {}

  virtual std::string getPackage() override { return package; }
  virtual IMapNode* getData() override { return children.get(); }
  virtual std::string getValue() override { return templ; }
  virtual ArrayNode* getLogs() override { return logs.get(); };

  virtual std::string getTestTemplate(std::string name) override {
    auto it = testTemplate.find(name);
    return (it == testTemplate.end()) ? "" : it->second;
  }

  virtual IUnitTestResultsNode* getResults() override {
    return resultsMap.get();
  }
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

  std::shared_ptr<VnVSpec> spec;
  std::shared_ptr<ArrayNode> children;
  std::shared_ptr<ArrayNode> unitTests;
  std::shared_ptr<MapNode> packages;
  std::shared_ptr<InfoNode> infoNode;
  std::shared_ptr<CommInfoNode> commInfo;

  VisitorLock* visitorLocking = nullptr;

  RootNode()
      : spec(new VnVSpec()),
        children(new ArrayNode()),
        unitTests(new ArrayNode()),
        packages(new MapNode()),
        infoNode(new InfoNode()),
        commInfo(new CommInfoNode()) {}

  virtual IMapNode* getPackages() override { return packages.get(); }
  virtual IArrayNode* getChildren() override { return children.get(); }
  virtual IArrayNode* getUnitTests() override { return unitTests.get(); }
  virtual IInfoNode* getInfoNode() override { return infoNode.get(); }
  virtual ICommInfoNode* getCommInfoNode() override { return commInfo.get(); }
  virtual long getTotalDuration() override { return duration; }

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

template <typename V> class Iterator {
 private:
  std::pair<V, long> current;

  std::stack<std::shared_ptr<DataBase>> stack;
  bool peaked = false;
  bool donedone = false;

  virtual void getLine(V& currentJson, long& currentValue) = 0;

 public:
  Iterator(){};
  virtual long streamId() const = 0;
  virtual bool hasNext() const = 0;

  virtual bool isDone() const { return donedone; };

  virtual std::pair<V, long> next() {
    pullLine(false);
    return current;
  }

  void pullLine(bool peek) {
    if (!peaked && hasNext()) {
      getLine(current.first, current.second);

      if (!current.first.contains(JSD::node)) {
        throw VnV::VnVExceptionBase("Stream sent info without node");
      }

      if ((current.first)[JSD::node].template get<std::string>() == JSN::done) {
        donedone = true;
      }
    }
    peaked = peek;
  }

  virtual long peekId() {
    pullLine(true);
    return current.second;
  };

  virtual ~Iterator(){};

  virtual void push(std::shared_ptr<DataBase> d) { stack.push(d); }
  virtual std::shared_ptr<DataBase> pop() {
    if (stack.size() > 0) {
      auto s = stack.top();
      stack.pop();
      return s;
    }
    return nullptr;
  }
  virtual std::shared_ptr<DataBase> top() {
    if (stack.size() > 0) {
      return stack.top();
    }
    return nullptr;
  }
};

template <typename T, typename V>
class MultiStreamIterator : public Iterator<V> {
  std::list<std::shared_ptr<T>> instreams;
  typename std::list<std::shared_ptr<T>>::iterator min;

  virtual void getLine(V& current, long& cid) override {
    min = std::min_element(
        instreams.begin(), instreams.end(),
        [](const std::shared_ptr<T>& x, const std::shared_ptr<T>& y) {
          if (x->isDone() && y->isDone())
            return (x->peekId() < y->peekId());
          else if (x->isDone())
            return false;
          else if (y->isDone())
            return true;

          return (x->peekId() < y->peekId());
        });
    auto p = (*min)->next();
    current = p.first;
    cid = p.second;
  }

 public:
  MultiStreamIterator() : Iterator<V>(){};

  std::list<std::shared_ptr<T>>& getInputStreams() { return instreams; }

  virtual void add(std::shared_ptr<T> iter) { instreams.push_back(iter); }

  virtual bool isDone() const {
    for (auto it : instreams) {
      if (!it->isDone()) {
        return false;
      }
    }
    return true;
  }

  virtual bool hasNext() const {
    for (auto it : instreams) {
      if (it->hasNext()) {
        return true;
      }
    }
    return false;
  }

  virtual long streamId() const override { return (*min)->streamId(); }

  virtual long peekId() {
    this->pullLine(true);
    return (*min)->peekId();
  }

  void push(std::shared_ptr<DataBase> d) override { (*min)->push(d); }

  std::shared_ptr<DataBase> pop() override { return (*min)->pop(); }

  std::shared_ptr<DataBase> top() override { return (*min)->top(); }
};

template <typename T> T WriteDataJson(IDataType_vec gather) {
  void* outdata;
  T main = T::array();
  for (int i = 0; i < gather.size(); i++) {
    IDataType_ptr d = gather[i];

    T cj = T::array();

    std::map<std::string, PutData> types = d->getLocalPutData();
    for (auto& it : types) {
      T childJson = T::object();

      PutData& p = it.second;
      outdata = d->getPutData(it.first);
      childJson[JSD::node] = JSN::shape;
      childJson[JSD::name] = p.name;
      childJson[JSD::shape] = p.shape;
      int count = std::accumulate(p.shape.begin(), p.shape.end(), 1,
                                  std::multiplies<>());

      switch (p.datatype) {
      case SupportedDataType::DOUBLE: {
        double* dd = (double*)outdata;
        if (p.shape.size() > 0) {
          std::vector<double> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::dtype] = JST::Double;

        break;
      }

      case SupportedDataType::LONG: {
        long* dd = (long*)outdata;
        if (p.shape.size() > 0) {
          std::vector<long> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::dtype] = JST::Long;

        break;
      }

      case SupportedDataType::STRING: {
        std::string* dd = (std::string*)outdata;
        if (p.shape.size() > 0) {
          std::vector<std::string> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::dtype] = JST::String;

        break;
      }

      case SupportedDataType::JSON: {
        json* dd = (json*)outdata;
        if (p.shape.size() > 0) {
          std::vector<json> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::sdt] = JST::Json;

        break;
      }
      }
      cj.push_back(childJson);
    }
    main.push_back(cj);
  }
  return main;
}

template <typename T> class StreamWriter {
 public:
  virtual void initialize(nlohmann::json& config, bool readMode) = 0;
  virtual void finalize(ICommunicator_ptr worldComm, long duration) = 0;
  virtual void newComm(long id, const T& obj, ICommunicator_ptr comm) = 0;
  virtual void write(long id, const T& obj, long jid) = 0;

  virtual bool fetch(long id, const json& obj, long timeoutInSeconds,
                     json& response) {
    return false;
  }

  virtual nlohmann::json getConfigurationSchema(bool readMode) = 0;

  StreamWriter(){};
  virtual ~StreamWriter(){

  };
};

template <typename T> class StreamManager : public OutputEngineManager {
  long vnvTimeStamp = 0;
  long recv = 0;

  long getNextId(ICommunicator_ptr comm, long myVal) {
    if (myVal > vnvTimeStamp) {
      vnvTimeStamp = myVal;
    }
    comm->AllReduce(&id, 1, &recv, sizeof(long), VnV::OpType::MAX);
    vnvTimeStamp = recv + 1;

    return recv;
  }

  static json getWorldRanks(ICommunicator_ptr comm, int root = 0) {
    std::vector<int> res(comm->Rank() == root ? comm->Size() : 0);
    int rank = comm->world()->Rank();
    comm->Gather(&rank, 1, res.data(), sizeof(int), root);
    json nJson = res;
    return nJson;
  }

 protected:
  bool inMemory;
  long id = 0;

  std::set<long> commids;
  std::string outputFile;
  std::shared_ptr<StreamWriter<T>> stream;

  std::string getId() { return std::to_string(StreamManager<T>::id++); }

  virtual void write(T& j) {
    j[JSD::duration] = VnV::RunTime::instance().duration();
    if (stream != nullptr) {
      stream->write(comm->uniqueId(), j, StreamManager<T>::id++);
    }
  }

  virtual void syncId() {
    StreamManager<T>::id = getNextId(comm, StreamManager<T>::id);
  }

  virtual void setComm(ICommunicator_ptr comm, bool syncIds) {
    setCommunicator(comm);
    long id = comm->uniqueId();

    if (commids.find(id) == commids.end()) {
      json procList = getWorldRanks(comm);

      if (comm->Rank() == getRoot()) {
        T nJson = T::object();
        nJson[JSD::name] = "comminfo";
        nJson[JSD::node] = JSN::commInfo;
        nJson[JSD::comm] = id;
        nJson[JSD::commList] = procList;

        nJson[JSD::duration] = VnV::RunTime::instance().duration();

        stream->newComm(id, nJson, comm);
      }
      commids.insert(id);
    }

    if (syncIds) {
      syncId();
    }
  }

 public:
  StreamManager(std::shared_ptr<StreamWriter<T>> stream_) : stream(stream_) {}

  virtual ~StreamManager() {}

  bool Fetch(const json& schema, long timeoutInSeconds,
             json& response) override {
    std::string line;
    std::size_t size;

    if (comm->Rank() == getRoot()) {
      response = json::object();
      bool worked =
          stream->fetch(comm->uniqueId(), schema, timeoutInSeconds, response);

      line = response.dump();
      size = (worked) ? line.size() : -1;
    }

    comm->BroadCast(&size, 1, sizeof(std::size_t), getRoot());
    if (size < 0) {
      return false;
    }

    line.resize(size);
    comm->BroadCast(const_cast<char*>(line.data()), size, sizeof(char),
                    getRoot());
    response = json::parse(line);
    return true;
  }

#define LTypes      \
  X(double, Double) \
  X(long long, Long) X(bool, Bool) X(std::string, String) X(json, Json)
#define X(typea, typeb)                                                     \
  void Put(std::string variableName, const typea& value, const MetaData& m) \
      override {                                                            \
    if (comm->Rank() == getRoot()) {                                        \
      T j = T::object();                                                    \
      j[JSD::name] = variableName;                                          \
      j[JSD::dtype] = JST::typeb;                                           \
      j[JSD::value] = value;                                                \
      j[JSD::shape] = json::object();                                       \
      j[JSD::node] = JSN::shape;                                            \
      j[JSD::meta] = m;                                                     \
      write(j);                                                             \
    }                                                                       \
  }
  LTypes
#undef X
#undef LTypes

      void
      Put(std::string variableName, IDataType_ptr data, const MetaData& m) {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::dataTypeStarted;
      j[JSD::name] = variableName;
      j[JSD::meta] = m;
      j[JSD::dtype] = data->getKey();
      write(j);
    }

    data->Put(this);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::dataTypeEnded;
      write(j);
    }
  }

  void PutGlobalArray(long long dtype, std::string variableName,
                      IDataType_vec data, std::vector<int> gsizes,
                      std::vector<int> sizes, std::vector<int> offset,
                      const MetaData& m) {
    VnV::DataTypeCommunication d(comm);

    // Gather all on the root processor
    IDataType_vec gather =
        d.GatherV(data, dtype, getRoot(), gsizes, sizes, offset, false);

    if (gather.size() > 0 && comm->Rank() == getRoot()) {
      json j;
      j[JSD::name] = variableName;
      j[JSD::node] = JSN::shape;
      j[JSD::shape] = gsizes;
      j[JSD::dtype] = JST::GlobalArray;
      j[JSD::key] = dtype;
      j[JSD::meta] = m;
      j[JSD::children] = WriteDataJson<json>(gather);
      write(j);
    }
  }

  void Log(ICommunicator_ptr logcomm, const char* package, int stage,
           std::string level, std::string message) {
    // Save the current communicator
    ICommunicator_ptr commsave = this->comm;

    // Set the logcomm as the current communication
    setComm(logcomm, true);

    // Write the log
    if (comm->Rank() == getRoot()) {
      T log = T::object();
      log[JSD::package] = package;
      log[JSD::stage] = stage;
      log[JSD::level] = level;
      log[JSD::message] = message;
      log[JSD::node] = JSN::log;
      log[JSD::name] = std::to_string(StreamManager<json>::id++);
      log[JSD::comm] = comm->uniqueId();
      write(log);
    }

    // Set the old comm
    if (commsave != nullptr) {
      setComm(commsave, false);
    }
  }

  json getConfigurationSchema(bool readMode) override {
    return stream->getConfigurationSchema(readMode);
  }

  void finalize(ICommunicator_ptr worldComm, long duration) override {
    setComm(worldComm, false);
    if (comm->Rank() == getRoot()) {
      T nJson = T::object();
      nJson[JSD::node] = JSN::duration;
      nJson[JSD::duration] = duration;
      write(nJson);
    }

    stream->finalize(worldComm, duration);
  }

  T getNodeMap(ICommunicator_ptr world) {
    std::string name = world->ProcessorName();
    auto h = StringUtils::simpleHash(name);
    std::vector<long long> results(world->Rank() == getRoot() ? world->Size()
                                                              : 0);
    world->Gather(&h, 1, results.data(), sizeof(long long), 0);
    if (world->Rank() == getRoot()) {
      T j = T::object();
      for (int i = 0; i < results.size(); i++) {
        std::string proc = std::to_string(results[i]);
        if (j.contains(proc)) {
          j[proc].push_back(i);
        } else {
          j[proc] = {i};
        }
      }
      return j;
    }
    return T::object();
  }

  void setFromJson(ICommunicator_ptr worldComm, json& config,
                   bool readMode) override {
    if (!readMode) {
      stream->initialize(config, readMode);
      setComm(worldComm, false);

      T node_map = getNodeMap(worldComm);

      if (comm->Rank() == getRoot()) {
        // This is the first one so we send over the info. .
        T nJson = T::object();
        nJson[JSD::title] = "VnV Simulation Report";
        nJson[JSD::date] =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count();
        nJson[JSD::name] = "MainInfo";
        nJson[JSD::node] = JSN::info;
        nJson[JSD::worldSize] = comm->Size();
        nJson[JSD::spec] = RunTime::instance().getFullJson();
        nJson[JSD::nodeMap] = node_map;
        nJson[JSD::mpiversion] = comm->VersionLibrary();
        nJson[JSD::prov] = RunTime::instance().getProv().toJson();

        write(nJson);
      }
    }
  }

  void file(ICommunicator_ptr comm, std::string packageName, std::string name,
            bool inputFile, std::string filename, std::string reader) {
    setComm(comm, false);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::file;
      j[JSD::input] = inputFile;

      ProvFile pf(filename, reader);
      pf.comm = comm->uniqueId();
      pf.package = packageName;
      pf.name = name;

      j[JSD::prov] = pf.toJson();
      write(j);
    }
  }

  void injectionPointEndedCallBack(std::string id, InjectionPointType type,
                                   std::string stageVal) override {
    syncId();
    if (comm->Rank() == getRoot()) {
      T j = T::object();

      j[JSD::node] = (type == InjectionPointType::End ||
                      type == InjectionPointType::Single)
                         ? JSN::injectionPointEnded
                         : JSN::injectionPointIterEnded;
      write(j);
    }
  }

  void injectionPointStartedCallBack(ICommunicator_ptr comm,
                                     std::string packageName, std::string id,
                                     InjectionPointType type,
                                     std::string stageVal, std::string filename,
                                     int line) override {
    setComm(comm, true);
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::name] = id;
      j[JSD::package] = packageName;
      j[JSD::comm] = comm->uniqueId();
      j[JSD::filename] = filename;
      j[JSD::line] = line;
      j[JSD::stageId] = stageVal;

      if (type == InjectionPointType::Begin ||
          type == InjectionPointType::Single) {
        j[JSD::node] = JSN::injectionPointStarted;
      } else {
        j[JSD::node] = JSN::injectionPointIterStarted;
      }
      write(j);
    }
  }
  void testStartedCallBack(std::string packageName, std::string testName,
                           bool internal, long uuid) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::name] = testName;
      j[JSD::package] = packageName;
      j[JSD::internal] = internal;
      j[JSD::node] = JSN::testStarted;
      j[JSD::testuid] = uuid;
      write(j);
    }
  }

  void testFinishedCallBack(bool result_) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::testFinished;
      j[JSD::result] = result_;

      write(j);
    }
  }

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                               std::string unitTestName) override {
    setComm(comm, true);

    if (comm->Rank() != getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::unitTestStarted;
      j[JSD::name] = unitTestName;
      j[JSD::package] = packageName;
      write(j);
    }
  }

  virtual void packageOptionsStartedCallBack(ICommunicator_ptr comm,
                                             std::string packageName) override {
    setComm(comm, true);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::packageOptionsStarted;
      j[JSD::package] = packageName;
      write(j);
    }
  }

  virtual void packageOptionsEndedCallBack(std::string packageName) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::packageOptionsFinished;
      write(j);
    }
  }

  void unitTestFinishedCallBack(IUnitTest* tester) {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::unitTestFinished;
      T c = T::array();
      for (auto it : tester->getResults()) {
        T kk = T::object();
        kk[JSD::name] = std::get<0>(it);
        kk[JSD::description] = std::get<1>(it);
        kk[JSD::result] = std::get<2>(it);
        c.push_back(kk);
      }
      j[JSD::results] = c;
      write(j);
    }
  }

  // IInternalOutputEngine interface
  std::string print() override { return "SS"; }
};

template <class T> class ParserVisitor : public VisitorLock {
 public:
  std::shared_ptr<Iterator<T>> jstream;
  std::shared_ptr<RootNode> rootInternal;
  long& idCounter;

  virtual std::pair<long, std::set<long>> visitCommNode(const T& j) {
    std::pair<long, std::set<long>> res;

    for (auto it : j[JSD::commList].items()) {
      res.second.insert(it.value().template get<long>());
    }

    res.first = j[JSD::comm].template get<long>();

    return res;
  }

  template <typename A> std::shared_ptr<A> mks_str(std::string noName = "") {
    std::shared_ptr<A> base_ptr(new A());
    base_ptr->name = noName;
    base_ptr->id = idCounter++;
    base_ptr->streamId = jstream->streamId();
    rootInternal->add(base_ptr);
    return base_ptr;
  }

  template <typename A> std::shared_ptr<A> mks(const T& j) {
    return mks_str<A>(j[JSD::name].template get<std::string>());
  }

  template <typename A, typename V>
  std::shared_ptr<A> visitShapeNode(const T& j) {
    std::shared_ptr<A> n = mks<A>(j);
    std::size_t shapeSize = j[JSD::shape].size();

    // Set the shape object.
    n->shape.reserve(shapeSize);
    for (auto it : j[JSD::shape].items()) {
      n->shape.push_back(it.value().template get<std::size_t>());
    }

    // Set the scalar value.
    if (shapeSize == 0) {
      n->value.reserve(1);
      n->value.push_back(j[JSD::value].template get<V>());
    } else {
      n->value.reserve(shapeSize);
      for (auto it : j[JSD::value].items()) {
        n->value.push_back(it.value().template get<V>());
      }
    }
    return n;
  }

  virtual std::shared_ptr<StringNode> visitStringNode(const T& j) {
    return visitShapeNode<StringNode, std::string>(j);
  }

  virtual std::shared_ptr<DoubleNode> visitDoubleNode(const T& j) {
    return visitShapeNode<DoubleNode, double>(j);
  }
  virtual std::shared_ptr<BoolNode> visitBoolNode(const T& j) {
    return visitShapeNode<BoolNode, bool>(j);
  };
  virtual std::shared_ptr<LongNode> visitLongNode(const T& j) {
    return visitShapeNode<LongNode, long long>(j);
  }

  virtual std::shared_ptr<JsonNode> visitJsonNode(const T& j) {
    auto n = mks<JsonNode>(j);

    std::size_t shapeSize = j[JSD::shape].size();
    n->shape.reserve(shapeSize);
    for (auto it : j[JSD::shape].items()) {
      n->shape.push_back(it.value().template get<std::size_t>());
    }
    if (shapeSize == 0) {
      n->value.push_back(j[JSD::value].dump());
    } else {
      n->value.reserve(j[JSD::value].size());
      for (auto it : j[JSD::value].items()) {
        n->value.push_back(it.value().dump());
      }
    }
    return n;
  }

  virtual std::shared_ptr<DataBase> visitGlobalArrayNode(const T& j) {
    auto n = mks<ShapeNode>(j);

    // Load the shape.
    for (auto it : j[JSD::shape].items()) {
      n->shape.push_back(it.value().template get<std::size_t>());
    }
    long long key = j[JSD::key].template get<long long>();

    int count = 0;
    n->value.reserve(j[JSD::children].size());
    for (auto& it : j[JSD::children].items()) {
      auto elm = mks_str<DataNode>(std::to_string(count++));
      elm->local = true;
      elm->key = key;

      // Push elm to the top of this stream.
      jstream->push(elm);
      int count1 = 0;
      for (auto& itt : it.value().items()) {
        visit(itt.value(), count1++);
      }
      jstream->pop();
      n->value.push_back(elm);
    }
    return n;
  }

  virtual std::shared_ptr<InfoNode> visitInfoNode(const T& j) {
    auto n = mks<InfoNode>(j);
    n->title = j[JSD::title].template get<std::string>();
    n->date = j[JSD::date].template get<long>();
    n->prov = std::make_shared<VnVProv>(j[JSD::prov]);

    rootInternal->spec.reset(new VnVSpec(j[JSD::spec]));
    rootInternal->commInfo->worldSize = j[JSD::worldSize].template get<int>();
    rootInternal->commInfo->nodeMap = j[JSD::nodeMap];
    rootInternal->commInfo->version =
        j[JSD::mpiversion].template get<std::string>();
    return n;
  };

  virtual void visitFileNode(const T& j) {
    ProvFile pf(j[JSD::prov]);

    bool input = j[JSD::input].template get<bool>();

    if (input) {
      rootInternal->infoNode->prov->addInputFile(pf);
    } else {
      rootInternal->infoNode->prov->addOutputFile(pf);
    }
  }

  virtual std::shared_ptr<LogNode> visitLogNode(const T& j) {
    auto n = mks<LogNode>(j);
    n->package = j[JSD::package].template get<std::string>();
    n->level = j[JSD::level].template get<std::string>();
    n->message = j[JSD::message].template get<std::string>();
    n->stage = std::to_string(j[JSD::stage].template get<int>());
    n->comm = std::to_string(j[JSD::comm].template get<long>());
    return n;
  };

  virtual std::shared_ptr<TestNode> visitPackageNodeStarted(const T& j) {
    auto n = mks_str<TestNode>("Information");
    n->package = j[JSD::package].template get<std::string>();
    n->templ = rootInternal->spec->package(n->package);
    return n;
  };

  virtual std::shared_ptr<TestNode> visitPackageNodeEnded(
      const T& j, std::shared_ptr<DataBase> node) {
    auto n = std::dynamic_pointer_cast<TestNode>(node);
    return n;
  };

  virtual std::shared_ptr<UnitTestNode> visitUnitTestNodeStarted(const T& j) {
    auto n = mks<UnitTestNode>(j);
    n->package = j[JSD::package].template get<std::string>();

    json templs = rootInternal->spec->unitTest(n->package, n->name);
    n->templ = templs["docs"];

    n->resultsMap = mks_str<UnitTestResultsNode>("results");

    for (auto it : templs["tests"].items()) {
      n->testTemplate[it.key()] = it.value().template get<std::string>();
    }
    return n;
  };

  virtual std::shared_ptr<UnitTestNode> visitUnitTestNodeEnded(
      const T& j, std::shared_ptr<DataBase> node) {
    auto n = std::dynamic_pointer_cast<UnitTestNode>(node);

    for (auto& it : j[JSD::results].items()) {
      auto r = mks<UnitTestResultNode>(it.value());
      r->desc = it.value()[JSD::description].template get<std::string>();
      r->result = it.value()[JSD::result].template get<bool>();
      n->resultsMap->m->add(r->name, r);
    }
    return n;
  }

  virtual std::shared_ptr<DataBase> visitDataNodeStarted(const T& j) {
    auto n = mks<DataNode>(j);
    n->key = j[JSD::dtype].template get<long long>();
    return n;
  }

  virtual std::shared_ptr<IDataNode> visitDataNodeEnded(
      const T& j, std::shared_ptr<DataBase> node) {
    return std::dynamic_pointer_cast<DataNode>(node);
  }

  virtual std::shared_ptr<TestNode> visitTestNodeStarted(const T& j) {
    auto n = mks<TestNode>(j);
    n->uid = j[JSD::testuid].template get<long>();
    n->package = j[JSD::package].template get<std::string>();
    n->templ = (j[JSD::internal].template get<bool>())
                   ? ""
                   : rootInternal->spec->test(n->package, n->name);
    n->internal = j[JSD::internal];
    return n;
  }

  virtual std::shared_ptr<TestNode> visitTestNodeIterStarted(
      const T& j, std::shared_ptr<TestNode> node) {
    return node;
  };

  virtual std::shared_ptr<TestNode> visitTestNodeIterEnded(
      const T& j, std::shared_ptr<DataBase> node) {
    return std::dynamic_pointer_cast<TestNode>(node);
  };

  virtual std::shared_ptr<TestNode> visitTestNodeEnded(
      const T& j, std::shared_ptr<DataBase> node) {
    auto n = std::dynamic_pointer_cast<TestNode>(node);
    n->result = j[JSD::result].template get<bool>();
    return n;
  };

  void addSource(const json& j, std::shared_ptr<InjectionPointNode> node) {
    if (j.contains(JSD::filename)) {
      std::string fname = j[JSD::filename].template get<std::string>();
      long line = j[JSD::line].template get<long>();
      std::string stage = j[JSD::stageId].template get<std::string>();
      node->addToSourceMap(stage, fname, line);
    }
  }

  virtual std::shared_ptr<InjectionPointNode> visitInjectionPointStartedNode(
      const T& j, long elementId) {
    auto n = mks<InjectionPointNode>(j);
    n->package = j[JSD::package].template get<std::string>();
    n->templ = rootInternal->spec->injectionPoint(n->package, n->name);
    n->commId = j[JSD::comm].template get<long>();
    n->startIndex = elementId;
    addSource(j, n);

    return n;
  }

  virtual std::shared_ptr<InjectionPointNode> visitInjectionPointEndedNode(
      const T& j, std::shared_ptr<DataBase> node, long elementId) {
    auto n = std::dynamic_pointer_cast<InjectionPointNode>(node);
    n->endIndex = elementId;
    return n;
  }

  virtual std::shared_ptr<InjectionPointNode>
  visitInjectionPointIterStartedNode(const T& j, std::shared_ptr<DataBase> node,
                                     long elementId) {
    auto n = std::dynamic_pointer_cast<InjectionPointNode>(node);
    addSource(j, n);
    return n;
  };

  virtual std::shared_ptr<InjectionPointNode> visitInjectionPointIterEndedNode(
      const T& j, std::shared_ptr<DataBase> node, long elementId) {
    auto n = std::dynamic_pointer_cast<InjectionPointNode>(node);
    return n;
  }

  ParserVisitor(std::shared_ptr<Iterator<T>> jstream_, long& idStart,
                std::shared_ptr<RootNode> rootNode)
      : jstream(jstream_), idCounter(idStart), rootInternal(rootNode) {
    rootInternal->visitorLocking = this;
  }

  virtual ~ParserVisitor() { rootInternal->visitorLocking = nullptr; }

  std::atomic_bool read_lock = ATOMIC_VAR_INIT(false);
  std::atomic_bool write_lock = ATOMIC_VAR_INIT(false);

  void process() {
    while (!jstream->isDone()) {
      if (jstream->hasNext()) {
        while (read_lock.load(std::memory_order_relaxed)) {
        }
        write_lock.store(true, std::memory_order_relaxed);
        auto p = jstream->next();
        visit(p.first, p.second);
        write_lock.store(false, std::memory_order_relaxed);
      }
    }
  }

  void lock() override {
    // Set the read lock, asking the other thread to stop writing.
    read_lock.store(true, std::memory_order_relaxed);

    // Wait for the other thread to release its write lock.
    while (write_lock.load(std::memory_order_relaxed)) {
    }
  }

  void release() override {
    // Release the read lock.
    read_lock.store(false, std::memory_order_relaxed);
  }

  void childNodeDispatcher(std::shared_ptr<DataBase> child, long elementId,
                           long duration) {
    std::shared_ptr<DataBase> parent = jstream->top();
    if (parent == nullptr) {
      parent = rootInternal;
    }

    DataBase::DataType ptype = parent->getType();
    DataBase::DataType ctype = child->getType();

    if (ptype == DataBase::DataType::Shape) {
      auto p = std::dynamic_pointer_cast<ShapeNode>(parent);
      p->getRawVector().push_back(child);

    } else if (ptype == DataBase::DataType::Test) {
      std::shared_ptr<ITestNode> p1 =
          std::dynamic_pointer_cast<ITestNode>(parent);
      if (ctype == DataBase::DataType::Log) {
        p1->getLogs()->add(child);
      } else {
        p1->getData()->add(child->getName(), child);
      }

    } else if (ptype == DataBase::DataType::UnitTest) {
      auto p = std::dynamic_pointer_cast<UnitTestNode>(parent);
      if (ctype == DataBase::DataType::Log) {
        p->getLogs()->add(child);
      } else {
        p->getData()->add(child->getName(), child);
      }

    } else if (ptype == DataBase::DataType::Data) {
      auto p = std::dynamic_pointer_cast<DataNode>(parent);
      if (ctype == DataBase::DataType::Log) {
        p->getLogs()->add(child);
      } else {
        p->getData()->add(child->getName(), child);
      }

    } else if (ptype == DataBase::DataType::InjectionPoint &&
               std::dynamic_pointer_cast<InjectionPointNode>(parent)
                   ->__isOpen) {
      auto p = std::dynamic_pointer_cast<InjectionPointNode>(parent);

      if (ctype == DataBase::DataType::Test) {
        if (child->getAsTestNode()->isInternal()) {
          p->internal = std::dynamic_pointer_cast<TestNode>(child);
        } else {
          p->getTests()->add(child);
        }

      } else if (ctype == DataBase::DataType::Log) {
        p->getLogs()->add(child);
        rootInternal->addIDN(child->getId(), child->getStreamId(),
                             node_type::LOG, elementId, duration, "Log");
      } else {
        throw VnVExceptionBase("Hmmmm");
      }

    } else if (ctype == DataBase::DataType::InjectionPoint) {
    } else if (ctype == DataBase::DataType::Log) {
      rootInternal->addIDN(child->getId(), child->getStreamId(), node_type::LOG,
                           elementId, duration, "Log");
    } else {
      throw VnVExceptionBase("Unsupported Parent element type");
    }
  }

  void visit(const T& j, long elementId) {
    std::string node = j[JSD::node].template get<std::string>();

    long duration = j[JSD::duration].template get<long>();

    if (node == JSN::duration) {
      rootInternal->duration = j[JSD::duration].template get<long>();

    } else if (node == JSN::done) {
      // DONE
    } else if (node == JSN::commInfo) {
      auto p = visitCommNode(j);
      rootInternal->getCommInfoNode()->getCommMap()->add(p.first, p.second);

    } else if (node == JSN::file) {
      visitFileNode(j);
    } else if (node == JSN::dataTypeEnded) {
      visitDataNodeEnded(j, jstream->pop());

    } else if (node == JSN::dataTypeStarted) {
      auto n = visitDataNodeStarted(j);
      childNodeDispatcher(n, elementId, duration);
      jstream->push(n);

    } else if (node == JSN::info) {
      rootInternal->infoNode = visitInfoNode(j);

    } else if (node == JSN::injectionPointEnded) {
      // This injection point is done.
      std::shared_ptr<InjectionPointNode> p =
          std::dynamic_pointer_cast<InjectionPointNode>(jstream->pop());
      p->__isOpen = false;
      visitInjectionPointEndedNode(j, p, elementId);

      rootInternal->addIDN(p->id, p->streamId, node_type::END, elementId,
                           duration, "End");

    } else if (node == JSN::injectionPointStarted) {
      std::shared_ptr<InjectionPointNode> p =
          visitInjectionPointStartedNode(j, elementId);

      p->__isOpen = true;
      childNodeDispatcher(p, elementId, duration);
      jstream->push(p);

      rootInternal->addIDN(p->id, p->streamId, node_type::START, elementId,
                           duration, "Begin");

    } else if (node == JSN::injectionPointIterEnded) {
      std::shared_ptr<InjectionPointNode> p =
          std::dynamic_pointer_cast<InjectionPointNode>(jstream->top());
      visitInjectionPointIterEndedNode(j, p, elementId);
      p->__isOpen = false;
      p->isIter = false;

    } else if (node == JSN::injectionPointIterStarted) {
      std::shared_ptr<InjectionPointNode> p =
          std::dynamic_pointer_cast<InjectionPointNode>(jstream->top());

      visitInjectionPointIterStartedNode(j, p, elementId);
      std::string stage = j[JSD::stageId].template get<std::string>();
      p->__isOpen = true;
      p->isIter = true;
      rootInternal->addIDN(p->id, p->streamId, node_type::ITER, elementId,
                           duration, stage);

    } else if (node == JSN::log) {
      std::shared_ptr<LogNode> n = visitLogNode(j);
      n->identity = elementId;
      childNodeDispatcher(n, elementId, duration);

    } else if (node == JSN::testFinished) {
      auto p = jstream->pop();
      auto pp = std::dynamic_pointer_cast<InjectionPointNode>(jstream->top());
      std::cout << pp->isIter << std::endl;

      if (std::dynamic_pointer_cast<InjectionPointNode>(jstream->top())
              ->isIter) {
        visitTestNodeIterEnded(j, p);
      } else {
        visitTestNodeEnded(j, p);
      }

    } else if (node == JSN::testStarted) {
      auto p = jstream->top();
      if (p->getType() != DataBase::DataType::InjectionPoint) {
        throw VnVExceptionBase("Bad Heirrarchy");
      }

      std::shared_ptr<InjectionPointNode> pp =
          std::dynamic_pointer_cast<InjectionPointNode>(p);

      if (pp->isIter) {
        long uid = j[JSD::testuid].template get<long>();
        auto tests = pp->getTests();

        if (j[JSD::internal].template get<bool>()) {
          auto t = std::dynamic_pointer_cast<TestNode>(pp->internal);
          if (t->uid != uid) {
            throw VnV::VnVExceptionBase("Wrong uid for internal test");
          }
          visitTestNodeIterStarted(j, t);
          jstream->push(t);
        } else {
          bool found = false;
          for (auto it : pp->tests->value) {
            auto t = std::dynamic_pointer_cast<TestNode>(it);
            if (t->uid == uid) {
              visitTestNodeIterStarted(j, t);
              jstream->push(t);
              found = true;
              break;
            }
          }
          if (!found) {
            std::cout << j.dump(2) << std::endl;
            throw VnV::VnVExceptionBase(
                "ERROR COULD NOT FIND TEST DURING ITERATION");
          }
        }
      } else {
        std::shared_ptr<TestNode> t = visitTestNodeStarted(j);
        childNodeDispatcher(t, elementId, duration);
        jstream->push(t);
      }

    } else if (node == JSN::unitTestFinished) {
      visitUnitTestNodeEnded(j, jstream->pop());

    } else if (node == JSN::unitTestStarted) {
      auto u = visitUnitTestNodeStarted(j);
      rootInternal->unitTests->add(u);
      jstream->push(u);

    } else if (node == JSN::packageOptionsStarted) {
      auto po = visitPackageNodeStarted(j);
      rootInternal->packages->add(po->package, po);
      jstream->push(po);
    } else if (node == JSN::packageOptionsFinished) {
      auto u = visitPackageNodeEnded(j, jstream->pop());
    } else if (node == JSN::shape) {
      std::string type = j[JSD::dtype].template get<std::string>();

      if (type == JST::String) {
        childNodeDispatcher(visitShapeNode<StringNode, std::string>(j),
                            elementId, duration);

      } else if (type == JST::Bool) {
        childNodeDispatcher(visitShapeNode<BoolNode, bool>(j), elementId,
                            duration);

      } else if (type == JST::Long) {
        childNodeDispatcher(visitShapeNode<LongNode, long>(j), elementId,
                            duration);

      } else if (type == JST::Double) {
        childNodeDispatcher(visitShapeNode<DoubleNode, double>(j), elementId,
                            duration);

      } else if (type == JST::Json) {
        childNodeDispatcher(visitJsonNode(j), elementId, duration);

      } else if (type == JST::GlobalArray) {
        childNodeDispatcher(visitGlobalArrayNode(j), elementId, duration);

      } else {
        throw VnVExceptionBase("Bad data type");
      }

    } else {
      throw VnVExceptionBase("Unrecognized Node Type");
    }
  }
};

constexpr const char* extension = ".fs";

template <typename T, typename V> class FileStream : public StreamWriter<V> {
 protected:
  std::string filestub;

  virtual ~FileStream() {}

  std::string getFileName(long id, bool makedir = true) {
    std::vector<std::string> fname = {std::to_string(id)};
    return getFileName_(filestub, {std::to_string(id) + extension}, true);
  }

  std::string getFileName_(std::string root, std::vector<std::string> fname,
                           bool mkdir) {
    fname.insert(fname.begin(), root);
    std::string filename = fname.back();
    fname.pop_back();
    std::string fullname = VnV::DistUtils::join(fname, 0777, mkdir);
    return VnV::DistUtils::join({fullname, filename}, 0777, false);
  }

 public:
  virtual void initialize(json& config, bool readMode) override {
    if (!readMode) {
      this->filestub = config["filename"].template get<std::string>();
    }
  }

  virtual nlohmann::json getConfigurationSchema(bool readMode) override {
    return readMode ? R"({})"_json : R"(
      { "type" : "object" , 
        "properties" : {
           "filename" : {"type":"string"}
        },
        "required" : ["filename"] 
      }
    )"_json;
  };

  virtual void finalize(ICommunicator_ptr wcomm, long duration) = 0;

  virtual void newComm(long id, const V& obj, ICommunicator_ptr comm) = 0;

  virtual void write(long id, const V& obj, long jid) = 0;

  static std::shared_ptr<IRootNode> parse(std::string file, long& id) {
    std::string filestub = file;

    auto stream = std::make_shared<MultiStreamIterator<T, V>>();

    std::string meta = VnV::DistUtils::join({filestub, ".meta"}, 0777, false);

    std::vector<std::string> files =
        VnV::DistUtils::listFilesInDirectory(filestub);

    for (auto& it : files) {
      std::cout << it << std::endl;
      std::size_t dot = it.find_first_of(".");
      try {
        if (it.substr(dot).compare(extension) == 0) {
          long id = std::atol(it.substr(0, dot).c_str());
          std::string fname = VnV::DistUtils::join({filestub, it}, 0777, false);
          stream->add(std::make_shared<T>(id, fname));
        }
      } catch (...) {
      }
    }

    std::shared_ptr<RootNode> root = std::make_shared<RootNode>();
    root->id = id++;
    root->name = "ROOT";

    ParserVisitor<V> visitor(stream, id, root);

    visitor.process();
    return root;
  }
};

}  // namespace Streaming
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
#endif