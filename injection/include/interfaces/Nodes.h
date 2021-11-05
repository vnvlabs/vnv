#ifndef NODES_H
#define NODES_H

#include <assert.h>

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "base/Provenance.h"
#include "base/exceptions.h"
#include "json-schema.hpp"

namespace VnV {

class IWalker;

namespace Nodes {

class MetaDataWrapper {
 public:
  std::map<std::string, std::string> m;

  std::string get(std::string key) {
    auto it = m.find(key);
    if (it != m.end()) {
      return it->second;
    }
    throw VnV::VnVExceptionBase("Bad MetaData Key");
  }
  bool has(std::string key) { return (m.find(key) != m.end()); }

  MetaDataWrapper& add(std::string key, std::string value) {
    m.insert(std::make_pair(key, value));
    return *this;
  }

  std::string asJson() {
    nlohmann::json j = m;
    return j.dump();
  }

  std::size_t size() { return m.size(); }

  json getAsDataChild() {
    json k = json::array();
    for (auto it : m) {
      k.push_back(it.first + ":" + it.second);
    }

    json j = json::object();
    j["text"] = "Meta Data Object";
    j["children"] = k;
    return j;
  }
};

#define DTYPES           \
  X(Bool, bool)          \
  X(Integer, int)        \
  X(Float, float)        \
  X(Double, double)      \
  X(String, std::string) \
  X(Json, std::string) X(Long, long) X(Shape, std::shared_ptr<DataBase>)
#define STYPES      \
  X(Array)          \
  X(Map)            \
  X(Log)            \
  X(InjectionPoint) \
  X(Info)           \
  X(CommInfo) X(Test) X(UnitTest) X(Data) X(UnitTestResult) X(UnitTestResults)
#define RTYPES X(Root)

#define X(x, y) class I##x##Node;
DTYPES
#undef X
#define X(x) class I##x##Node;
STYPES
RTYPES
#undef X

class DataBase {
 public:
  enum class DataType {
#define X(x, y) x,
    DTYPES
#undef X

#define X(x) x,
        STYPES
#undef X

            Root
  };
  bool _open = false;
  long id;
  long streamId;

  MetaDataWrapper metaData;
  std::string name;  // name can be assigned, default is to use id (which is unique).
  DataType dataType;

  DataBase(DataType type);
  virtual ~DataBase();

#define X(x, y) virtual I##x##Node* getAs##x##Node();
  DTYPES
#undef X
#define X(x) virtual I##x##Node* getAs##x##Node();
  STYPES
  RTYPES
#undef X

  virtual long getId();
  virtual std::string getName();
  virtual std::string getTypeStr();
  virtual MetaDataWrapper& getMetaData() { return metaData; }
  virtual bool check(DataType type);
  virtual DataType getType();

  virtual bool open() { return _open; }
  virtual void open(bool value) { _open = value; }

  virtual long getStreamId() { return streamId; }

  virtual void registerChildren(long& idCounter, IRootNode* rootNode) {}

  virtual std::string toString() {
    std::ostringstream oss;
    oss << "Node: " << id << "Name: " << name << "DataType: " << getTypeStr();
    return oss.str();
  }

  std::string getDataChildren(int fileId, int level) { return getDataChildren_(fileId, level).dump(); }

  virtual json getDataChildren_(int fileId, int level) {
    json a = json::array();
    a.push_back("Name: " + getName());
    a.push_back("Type: " + getTypeStr());
    a.push_back("ID: " + std::to_string(getId()));
    if (metaData.size() > 0) {
      a.push_back(metaData.getAsDataChild());
    }
    return a;
  }

  virtual json getAsDataChild(int fileId, int level) {
    json j = json::object();
    j["text"] = getName();

    if (level > 0) {
      j["children"] = getDataChildren_(fileId, level - 1);
    } else {
      j["children"] = true;
    }

    j["li_attr"] = json::object();
    j["li_attr"]["nodeId"] = getId();
    j["li_attr"]["fileId"] = fileId;
    return j;
  }

  
};

#define X(X, x)                                                                                 \
  class I##X##Node : public DataBase {                                                          \
   public:                                                                                      \
    I##X##Node();                                                                               \
    virtual const std::vector<std::size_t>& getShape() = 0;                                     \
    virtual x getValueByShape(const std::vector<std::size_t>& shape) = 0;                       \
    virtual x getValueByIndex(const std::size_t ind) = 0;                                       \
    std::string valueToString(x ind);                                                           \
    virtual std::vector<x>& getRawVector() = 0;                                                 \
    virtual x getScalarValue() = 0;                                                             \
    virtual int getNumElements() = 0;                                                           \
    virtual ~I##X##Node();                                                                      \
    virtual std::string getShapeJson() {                                                        \
      nlohmann::json j = this->getShape();                                                      \
      return j.dump();                                                                          \
    }                                                                                           \
                                                                                                \
    virtual json getDataChildren_(int fileId, int level) override {                             \
      json a = DataBase::getDataChildren_(fileId, level);                                       \
      a.push_back("Dimension:" + std::to_string(getShape().size()));                            \
      a.push_back("Size:" + std::to_string(getNumElements()));                                  \
      a.push_back("Shape: " + getShapeJson());                                                  \
      if (getShape().size() == 0) {                                                             \
        a.push_back("Value:" + valueToString(getScalarValue()));                                \
      } else {                                                                                  \
        json shapes = json::array();                                                            \
                                                                                                \
        auto shape = getShape();                                                                \
        std::vector<std::size_t> counters(shape.size(), 0);                                     \
                                                                                                \
        bool done = false;                                                                      \
        while (!done) {                                                                         \
          json countersj = counters;                                                            \
          shapes.push_back(countersj.dump() + ": " + valueToString(getValueByShape(counters))); \
          for (int i = counters.size() - 1; i > -1; i--) {                                      \
            counters[i] = counters[i] + 1;                                                      \
            if (counters[i] == shape[i]) {                                                      \
              if (i == 0) {                                                                     \
                done = true;                                                                    \
              }                                                                                 \
              counters[i] = 0;                                                                  \
            } else {                                                                            \
              break;                                                                            \
            }                                                                                   \
          }                                                                                     \
        }                                                                                       \
        json vals = json::object();                                                             \
        vals["text"] = "Values";                                                                \
        vals["children"] = shapes;                                                              \
      }                                                                                         \
      return a;                                                                                 \
    }                                                                                           \
  };
DTYPES
#undef X

// ArrayNode is an array of DataNodes.
class IArrayNode : public DataBase {
 public:
  IArrayNode();
  virtual DataBase* get(std::size_t idx) { return getShared(idx).get(); };
  virtual std::size_t size() = 0;
  virtual IArrayNode* add(std::shared_ptr<DataBase> data) = 0;
  virtual std::shared_ptr<DataBase> getShared(std::size_t idx) = 0;
  virtual ~IArrayNode();

  // Get the value for inserting into a text object.
  virtual std::string getValue() = 0;

  void iter(std::function<void(DataBase*)>& lambda);

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Size: " + std::to_string(size()));
    for (int i = 0; i < size(); i++) {
      j.push_back(get(i)->getAsDataChild(fileId, level - 1));
    }
    return j;
  }
};

class IMapNode : public DataBase {
 public:
  IMapNode();
  virtual IArrayNode* get(std::string key) = 0;
  virtual bool contains(std::string key) = 0;
  virtual std::vector<std::string> fetchkeys() = 0;
  virtual std::size_t size() = 0;

  virtual DataBase* get(std::string key, std::size_t index) {
    auto a = get(key);
    if (a == nullptr) {
      throw VnVExceptionBase("Key %s does not exist in map", key.c_str());
    } else if (index > a->size()) {
      throw VnVExceptionBase("Index out of range error (index:%d, size: %d)", index, a->size());
    }
    return a->get(index);
  }

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    for (auto it : fetchkeys()) {
      j.push_back(get(it)->getAsDataChild(fileId, level - 1));
    }
    return j;
  }

  virtual std::string getValue() = 0;
  virtual ~IMapNode();
};

class IDataNode : public DataBase {
 public:
  IDataNode();
  virtual IArrayNode* getLogs() = 0;
  virtual IMapNode* getData() = 0;
  virtual long long getDataTypeKey() = 0;
  virtual std::string getValue() = 0;
  virtual bool getLocal() = 0;
  virtual ~IDataNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back(getLogs()->getAsDataChild(fileId, level - 1));
    j.push_back(getData()->getAsDataChild(fileId, level - 1));
    j.push_back("Data Type Key:" + std::to_string(getDataTypeKey()));
    j.push_back("Local:" + std::to_string(getLocal()));
    return j;
  }

  virtual void open(bool value) override;
};

class IInfoNode : public DataBase {
 public:
  IInfoNode();
  virtual std::string getTitle() = 0;
  virtual long getDate() = 0;
  virtual VnV::VnVProv* getProv() = 0;
  virtual std::string getValue() = 0;

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Title:" + getTitle());
    j.push_back("Date:" + std::to_string(getDate()));
    j.push_back("Date:" + std::to_string(getDate()));
    j.push_back("Prov: TODO");
    return j;
  }

  virtual ~IInfoNode();
};

class ICommMap {
 public:
  ICommMap() {}
  virtual void add(long id, std::set<long>& comms) = 0;
  virtual nlohmann::json toJson(bool strip) = 0;

  virtual bool commContainsProcessor(long commId, long proc) = 0;
  virtual bool commContainsComm(long commParent, long commChild) = 0;
  virtual std::set<long> commChain(long comm) = 0;
  virtual bool commIsChild(long parentId, long childId) = 0;
  virtual bool commIsSelf(long streamId, long proc) = 0;
  virtual bool commsIntersect(long streamId, long comm) = 0;

  virtual nlohmann::json listComms() = 0;

  virtual ~ICommMap() {}

  virtual std::string getComms() { return listComms().dump(); }
  virtual std::string toJsonStr(bool strip) = 0;
};

class ICommInfoNode : public DataBase {
 public:
  ICommInfoNode();
  virtual int getWorldSize() = 0;
  virtual ICommMap* getCommMap() = 0;
  virtual std::string getNodeMap() = 0;
  virtual std::string getVersion() = 0;

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("World Size:" + std::to_string(getWorldSize()));
    j.push_back("Version:" + getVersion());
    return j;
  }

  virtual ~ICommInfoNode();
};

class FetchRequest {
 public:
  std::string schema;
  long id;
  long jid;
  long expiry;
  std::string message;

  FetchRequest(std::string s, long id_, long jid_, long e, std::string m)
      : schema(s), id(id_), jid(jid_), expiry(e), message(m) {}
  std::string getSchema() { return schema; }
  long getExpiry() { return expiry; }
  long getId() { return id; }
  long getJID() { return jid; }
  std::string getMessage() { return message; }
};

class ITestNode : public DataBase {
  std::shared_ptr<FetchRequest> fetch = nullptr;

 public:
  ITestNode();

  virtual FetchRequest* getFetchRequest() {
    if (fetch != nullptr) {
      return fetch.get();
    }
    return nullptr;
  }
  virtual void resetFetchRequest() { fetch.reset(); }
  virtual void setFetchRequest(std::string schema, long id, long jid, long expiry, std::string message) {
    fetch.reset(new FetchRequest(schema, id, jid, expiry, message));
  }

  virtual std::string getPackage() = 0;
  virtual IMapNode* getData() = 0;
  virtual std::string getValue() = 0;
  virtual IArrayNode* getLogs() = 0;
  virtual bool isInternal() = 0;
  virtual ~ITestNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Package:" + getPackage());
    j.push_back("Internal:" + std::to_string(isInternal()));
    j.push_back(getLogs()->getAsDataChild(fileId, level - 1));
    j.push_back(getData()->getAsDataChild(fileId, level - 1));
    return j;
  }

  virtual void open(bool value) override;
};

class IInjectionPointNode : public DataBase {
 public:
  IInjectionPointNode();
  virtual long getStartIndex() = 0;
  virtual long getEndIndex() = 0;
  virtual std::string getComm() = 0;
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getTests() = 0;
  virtual std::string getValue() = 0;
  virtual IArrayNode* getLogs() = 0;
  virtual ITestNode* getData() = 0;
  virtual std::string getSourceMap() = 0;
  virtual ~IInjectionPointNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Package:" + getPackage());
    j.push_back("Comm:" + getComm());
    j.push_back("SourceMap:" + getSourceMap());
    j.push_back(getLogs()->getAsDataChild(fileId, level - 1));
    j.push_back(getData()->getAsDataChild(fileId, level - 1));
    j.push_back(getTests()->getAsDataChild(fileId, level - 1));

    return j;
  }

  virtual void open(bool value) override;
};

class ILogNode : public DataBase {
 public:
  ILogNode();
  virtual std::string getPackage() = 0;
  virtual std::string getLevel() = 0;
  virtual std::string getMessage() = 0;
  virtual std::string getValue() = 0;
  virtual std::string getStage() = 0;
  virtual std::string getComm() = 0;
  virtual ~ILogNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Package:" + getPackage());
    j.push_back("Level:" + getLevel());
    j.push_back("Message:" + getMessage());
    j.push_back("Stage:" + getStage());
    j.push_back("Value:" + getValue());
    j.push_back("Comm:" + getComm());
    return j;
  }
};

class IUnitTestResultNode : public DataBase {
 public:
  IUnitTestResultNode();
  virtual std::string getTemplate() = 0;
  virtual std::string getDescription() = 0;
  virtual bool getResult() = 0;
  virtual ~IUnitTestResultNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Description:" + getDescription());
    j.push_back("Result:" + std::to_string(getResult()));
    return j;
  }
};

class IUnitTestResultsNode : public DataBase {
 public:
  IUnitTestResultsNode();
  virtual IUnitTestResultNode* get(std::string key) = 0;
  virtual bool contains(std::string key) = 0;
  virtual std::vector<std::string> fetchkeys() = 0;
  virtual ~IUnitTestResultsNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    for (auto it : fetchkeys()) {
      j.push_back(get(it)->getAsDataChild(fileId, level));
    }
    return j;
  }
};

class IUnitTestNode : public DataBase {
 public:
  IUnitTestNode();
  virtual std::string getPackage() = 0;
  virtual IMapNode* getData() = 0;
  virtual IArrayNode* getLogs() = 0;
  virtual IUnitTestResultsNode* getResults() = 0;
  virtual std::string getValue() = 0;
  virtual std::string getTestTemplate(std::string name) = 0;
  virtual ~IUnitTestNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Package:" + getPackage());
    j.push_back(getResults()->getAsDataChild(fileId, level - 1));
    j.push_back(getLogs()->getAsDataChild(fileId, level - 1));
    j.push_back(getData()->getAsDataChild(fileId, level - 1));
    return j;
  }

  virtual void open(bool value) override;
};

class VnVSpec {
  nlohmann::json spec;

  std::string getter(std::string r, std::string key) const {
    try {
      return spec[r][key]["docs"].get<std::string>();
    } catch (...) {
      throw VnV::VnVExceptionBase("%s:%s:docs does not exist or is not a string ", r.c_str(), key.c_str());
    }
  }

 public:
  VnVSpec(const nlohmann::json& j) : spec(j) {}

  template <typename T> VnVSpec(const T& j) : spec(nlohmann::json::parse(j.dump())){};

  VnVSpec() {}
  void set(const nlohmann::json& s) { spec = s; }

  std::string get() { return spec.dump(); }

  std::string intro() {
    try {
      return spec["Introduction"]["docs"].get<std::string>();
    } catch (...) {
      throw VnV::VnVExceptionBase("No introduction available");
    }
  }

  std::string conclusion() {
    try {
      return spec["Conclusion"]["docs"].get<std::string>();
    } catch (...) {
      throw VnV::VnVExceptionBase("No conclusion available");
    }
  }

  std::string injectionPoint(std::string package, std::string name) const {
    return getter("InjectionPoints", package + ":" + name);
  }

  std::string dataType(std::string key) const { return getter("DataTypes", key); }
  std::string test(std::string package, std::string name) const { return getter("Tests", package + ":" + name); }

  std::string file(std::string package, std::string name) const { return getter("Files", package + ":" + name); }

  std::string action(std::string package, std::string name) const {
    std::cout << package << ":" << name << std::endl;
    return getter("Actions", package + ":" + name);
  }

  std::string package(std::string package) const { return getter("Options", package); }

  nlohmann::json unitTest(std::string package, std::string name) const {
    return spec["UnitTests"][package + ":" + name];
  }
};

enum class node_type { ROOT, POINT, START, ITER, END, LOG, WAITING, DONE };

class WalkerNode {
 public:
  Nodes::DataBase* item;
  Nodes::node_type type;
  std::set<long> edges;
  long long time;
};

class WalkerWrapper {
  std::shared_ptr<IWalker> ptr;
  std::shared_ptr<WalkerNode> node;
  IRootNode* rootNode;

 public:
  WalkerWrapper(std::shared_ptr<IWalker> walker, IRootNode* root);
  WalkerNode* next();
};

class IDN {
 public:
  long id;
  long streamId;
  node_type type;
  long duration;
  std::string stage;

  IDN(long a, long b, node_type c, long d, std::string e) : id(a), streamId(b), type(c), duration(d), stage(e) {}
  IDN() : id(-1), streamId(-1), type(node_type::ROOT), duration(-1), stage("") {}
};

class IRootNode : public DataBase {
  // index -> streamId -> nodeId
  std::map<long, std::list<IDN>> nodes;
  std::map<long, std::shared_ptr<DataBase>> idMap;

  std::set<IWalker*> walkers;

 public:
  IRootNode();

  virtual DataBase* findById(long id) {
    if (id == this->id) {
      return this;
    }

    auto it = idMap.find(id);
    if (it != idMap.end()) {
      return it->second.get();
    }
    throw VnV::VnVExceptionBase("Invalid Id %s", id);
  }

  void registerWalkerCallback(IWalker* walker) { walkers.insert(walker); }
  void deregisterWalkerCallback(IWalker* walker) { walkers.erase(walker); }

  virtual void add(std::shared_ptr<DataBase> ptr) { idMap[ptr->getId()] = ptr; }

  virtual void addIDN(long id, long streamId, node_type type, long index, long duration, std::string stage);

  virtual IArrayNode* getChildren() = 0;
  virtual IArrayNode* getUnitTests() = 0;
  virtual IInfoNode* getInfoNode() = 0;
  virtual ICommInfoNode* getCommInfoNode() = 0;
  virtual IMapNode* getPackages() = 0;
  virtual long getTotalDuration() = 0;

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Duration:" + std::to_string(getTotalDuration()));
    j.push_back(getPackages()->getAsDataChild(fileId, level - 1));
    j.push_back(getActions()->getAsDataChild(fileId, level - 1));
    j.push_back(getUnitTests()->getAsDataChild(fileId, level - 1));
    j.push_back(getInfoNode()->getAsDataChild(fileId, level - 1));
    j.push_back(getChildren()->getAsDataChild(fileId, level - 1));

    json jj = json::object();
    jj["text"] = "Injection Points and Log Messages";

    json ch = json::array();
    for (auto it : nodes) {
      for (auto itt : it.second) {
        if (itt.type == node_type::START || itt.type == node_type::POINT || itt.type == node_type::LOG) {
          ch.push_back(findById(itt.id)->getAsDataChild(fileId, 0));
        }
      }
    }
    jj["children"] = ch;
    j.push_back(jj);

    return j;
  }

  virtual ITestNode* getPackage(std::string package) { return getPackages()->get(package, 0)->getAsTestNode(); }

  virtual ITestNode* getAction(std::string package) { return getActions()->get(package, 0)->getAsTestNode(); }

  virtual void lock() = 0;

  virtual IMapNode* getActions() = 0;

  virtual void release() = 0;

  virtual bool hasId(long id) { return findById(id) != NULL; }

  virtual const VnVSpec& getVnVSpec() = 0;

  virtual bool processing() const = 0;

  virtual void respond(long id, long jid, const std::string& response) = 0;

  virtual std::map<long, std::list<IDN>>& getNodes() { return nodes; }

  WalkerWrapper getWalker(std::string package, std::string name, std::string config);

  virtual ~IRootNode();
};

}  // namespace Nodes

}  // namespace VnV
#endif  // NODES_H