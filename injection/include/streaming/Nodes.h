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
#include <thread>
#include <vector>

#include "base/Provenance.h"
#include "base/exceptions.h"
#include "json-schema.hpp"
#include "streaming/IDN.h"

#define ID_NOT_INITIALIZED_YET -1000

#define GETTERSETTER(NAME, type)                          \
  void set##NAME(const type& NAME) { this->NAME = NAME; } \
  const type& get##NAME() { return this->NAME; }

namespace VnV {

class IWalker;

namespace Nodes {

class VisitorLock {
 public:
  VisitorLock() {}
  virtual ~VisitorLock() {}
  virtual void lock() = 0;
  virtual void release() = 0;
};

class MetaDataWrapper {
  std::map<std::string, std::string> m;

 public:
  std::string const get(std::string key) {
    auto it = m.find(key);
    if (it != m.end()) {
      return it->second;
    }
    throw INJECTION_EXCEPTION("Metadata key %s does not exist", key.c_str());
  }
  bool has(std::string key) const { return (m.find(key) != m.end()); }

  MetaDataWrapper& add(std::string key, std::string value) {
    m.insert(std::make_pair(key, value));
    return *this;
  }

  void fromJson(json& val) {
    m.clear();
    for (auto it : val.items()) {
      m[it.key()] = it.value().get<std::string>();
    }
  }

  std::string asJson() const {
    nlohmann::json j = m;
    return j.dump();
  }

  json toJson() const {
    return m;
  }

  std::size_t size() const { return m.size(); }

  json getAsDataChild() const {
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
  X(Json, std::string)   \
  X(Long, long)          \
  X(Shape, std::shared_ptr<DataBase>)
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

 private:
  DataType dataType;
 protected:
  IRootNode* _rootNode = nullptr;

  IRootNode* rootNode() {
    return _rootNode;
  }

  long id = ID_NOT_INITIALIZED_YET;
 public:
  
  DataBase(DataType type);
  virtual ~DataBase();

#define X(x, y) virtual std::shared_ptr<I##x##Node> getAs##x##Node(std::shared_ptr<DataBase> ptr);
  DTYPES
#undef X
#define X(x) virtual std::shared_ptr<I##x##Node> getAs##x##Node(std::shared_ptr<DataBase> ptr);
  STYPES
  RTYPES
#undef X

  virtual std::string getUsageType() {
    return getTypeStr();
  }

  virtual long getId() { return id; } 
  virtual const MetaDataWrapper& getMetaData() = 0;
  virtual long getStreamId() = 0;

  virtual bool getopen() = 0;
  virtual void setopen(bool value) = 0;

  virtual std::string getNameInternal() = 0;

  virtual DataType getType();
  virtual std::string getName();
  virtual std::string getTypeStr();

  static DataBase::DataType getDataTypeFromString(std::string s);

  virtual void open(bool value) { setopen(value); }

  virtual bool check(DataType type);

  virtual void setRootNode(long id, IRootNode* rootNode) {
    this->_rootNode = rootNode;
    this->id = id;
  }

  virtual std::string toString() {
    std::ostringstream oss;
    oss << "Node: " << getId() << "Name: " << getName() << "DataType: " << getTypeStr();
    return oss.str();
  }

  std::string getDataChildren(int fileId, int level) { return getDataChildren_(fileId, level).dump(); }

  virtual json getDataChildren_(int fileId, int level) {
    json a = json::array();
    a.push_back("Name: " + getName());
    a.push_back("Type: " + getTypeStr());
    a.push_back("ID: " + std::to_string(getId()));
    if (getMetaData().size() > 0) {
      a.push_back(getMetaData().getAsDataChild());
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
    virtual x getScalarValue() = 0;                                                             \
    virtual int getNumElements() = 0;                                                           \
    virtual ~I##X##Node();                                                                      \
    virtual std::string getShapeJson() {                                                        \
      nlohmann::json j = this->getShape();                                                      \
      return j.dump();                                                                          \
    }                                                                                           \
    virtual void add(const x & s) = 0 ;                                                         \
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
  virtual std::shared_ptr<DataBase> get(std::size_t idx) = 0;
  virtual std::size_t size() = 0;
  virtual void add(std::shared_ptr<DataBase> data) = 0;
  virtual ~IArrayNode();

  void iter(std::function<void(std::shared_ptr<DataBase>)>& lambda);

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Size: " + std::to_string(size()));
    for (int i = 0; i < size(); i++) {
      j.push_back(get(i)->getAsDataChild(fileId, level - 1));
    }
    return j;
  }

  virtual std::string toString() override {
    std::ostringstream oss;
    oss << "[" ;
    for (int i = 0; i < size(); i++ ) {
      oss << ((i==0) ? "":"," ) << get(i)->toString();
    }
    oss << "]";
    return oss.str();
  }
};

class IMapNode : public DataBase {
 public:
  IMapNode();
  virtual std::shared_ptr<IArrayNode> get(std::string key)= 0; 
  
  virtual void insert(std::string key, std::shared_ptr<DataBase> val) = 0;

  virtual bool contains(std::string key) = 0;
  virtual std::vector<std::string> fetchkeys() = 0;
  virtual std::size_t size() = 0;

  virtual std::shared_ptr<DataBase> get(std::string key, std::size_t index) {
    auto a = get(key);
    if (a == nullptr) {
      throw INJECTION_EXCEPTION("Key %s does not exist in map", key.c_str());
    } else if (index > a->size()) {
      throw INJECTION_EXCEPTION("Index out of range error (index:%d, size: %d)", index, a->size());
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

  virtual std::string toString() override {
    std::ostringstream oss;
    oss << "{";
    auto keys = fetchkeys();
    for (int i = 0; i < keys.size(); i++) {
      oss << ( (i==0)?"":",") << keys[i] << " : " <<  get(keys[i])->toString();
    }
    oss << "}";
    return oss.str();
  }

  virtual ~IMapNode();
};

class IDataNode : public DataBase {
 public:
  IDataNode();
  virtual std::shared_ptr<IArrayNode> getLogs() = 0;
  virtual std::shared_ptr<IMapNode> getData() = 0;
  virtual long long getDataTypeKey() = 0;
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

  virtual std::string toString() override {
    return getData()->toString();
  }

  virtual void open(bool value) override;
};

class IInfoNode : public DataBase {
 protected:
  virtual std::shared_ptr<VnV::VnVProv> getProvInternal() = 0;
  
 
 public:
  IInfoNode();
  virtual std::string getTitle() = 0;
  virtual long getStartTime() = 0;
  virtual long getEndTime() = 0;


  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Title:" + getTitle());
    j.push_back("Start:" + std::to_string(getStartTime()));
    j.push_back("End:" + std::to_string(getEndTime()));
    j.push_back("Durr:" + std::to_string(getEndTime()-getStartTime()));
    j.push_back("Prov: TODO");
    return j;
  }

  virtual void addInputFile(std::shared_ptr<ProvFile> pv) {
    getProvInternal()->addInputFile(pv);
  };
  virtual void addOutputFile(std::shared_ptr<ProvFile> pv) {
    getProvInternal()->addOutputFile(pv);
  } 

  std::shared_ptr<VnV::VnVProv> getProv() {
    return getProvInternal();
  }

  virtual ~IInfoNode();
};

class ICommMap {
 public:
  ICommMap() {}
  virtual void add(long id, const std::set<long>& comms) = 0;
  virtual nlohmann::json toJson(bool strip) const = 0;

  virtual bool commContainsProcessor(long commId, long proc) const = 0;
  virtual bool commContainsComm(long commParent, long commChild) const = 0;
  virtual std::set<long> commChain(long comm) const  = 0;
  virtual bool commIsChild(long parentId, long childId) const = 0;
  virtual bool commIsSelf(long streamId, long proc) const = 0;
  virtual bool commsIntersect(long streamId, long comm) const = 0;

  virtual nlohmann::json listComms() const = 0;

  virtual ~ICommMap() {}

  virtual std::string getComms() const { return listComms().dump(); }
  virtual std::string toJsonStr(bool strip) const = 0;
};


class ICommInfoNode : public DataBase {
 protected:
    
  virtual std::shared_ptr<ICommMap> getCommMapInternal() = 0;
  
 public:
 
  ICommInfoNode();
  virtual int getWorldSize() = 0;
  
  virtual std::shared_ptr<const ICommMap> getCommMap() {
    return getCommMapInternal();
  };
  
  virtual void add(long id, const std::set<long>& comms) {
    getCommMapInternal()->add(id,comms);
  }

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
public:
  enum class TestNodeUsage { TEST, INTERNAL, ACTION, PACKAGE};
  static std::string getUsageString(TestNodeUsage u) {
    switch (u) {
      case TestNodeUsage::ACTION: return "Action";
      case TestNodeUsage::INTERNAL: return "Internal";
      case TestNodeUsage::TEST: return "Test";
      case TestNodeUsage::PACKAGE: return "Package";
    }
    // We should test this never happens 
    assert(false &&  "Error: Forget to add toString condition int testNodeUsage");
    std::abort();
  }

  static TestNodeUsage getUsageFromString(std::string u) {
      if     (u.compare("Action") == 0 ) return TestNodeUsage::ACTION;
      else if(u.compare("Internal") == 0 ) return TestNodeUsage::INTERNAL;
      else if(u.compare("Test") == 0 ) return TestNodeUsage::TEST;
      else if(u.compare("Package") == 0 ) return TestNodeUsage::PACKAGE;
      throw INJECTION_EXCEPTION("%s is not a valid string representation of any known enum in enum class TestNodeUsage ", u.c_str());
  }

 protected: 
  std::shared_ptr<FetchRequest> fetch = nullptr;
  TestNodeUsage usage = TestNodeUsage::TEST;

 public:


  virtual std::string getUsageType() override {
    return getUsageString(getUsage());
  }    

  ITestNode();

  virtual std::shared_ptr<FetchRequest> getFetchRequest() {
    if (fetch != nullptr) {
      return fetch;
    }
    return nullptr;
  }

  virtual void resetFetchRequest() { fetch.reset(); }

  virtual void setFetchRequest(std::string schema, long id, long jid, long expiry, std::string message) {
    fetch.reset(new FetchRequest(schema, id, jid, expiry, message));
  }

  virtual TestNodeUsage getUsage()  = 0 ; 
  virtual std::string getPackage() = 0;
  virtual std::shared_ptr<IMapNode> getData() = 0;
  virtual std::shared_ptr<IArrayNode> getLogs() = 0;
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
  virtual long getStartTime() = 0;
  virtual long getEndTime() = 0;

  virtual std::string getComm() = 0;
  virtual std::string getPackage() = 0;
  virtual std::shared_ptr<IArrayNode> getTests() = 0;
  virtual std::shared_ptr<IArrayNode> getLogs() = 0;
  virtual std::shared_ptr<ITestNode> getData() = 0;
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
  virtual std::string getStage() = 0;
  virtual std::string getComm() = 0;
  virtual ~ILogNode();

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Package:" + getPackage());
    j.push_back("Level:" + getLevel());
    j.push_back("Message:" + getMessage());
    j.push_back("Stage:" + getStage());
    j.push_back("Comm:" + getComm());
    return j;
  }
};

class IUnitTestResultNode : public DataBase {
 public:
  IUnitTestResultNode();
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
  virtual std::shared_ptr<IUnitTestResultNode> get(std::string key) = 0;
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
  virtual std::shared_ptr<IMapNode> getData() = 0;
  virtual std::shared_ptr<IArrayNode> getLogs() = 0;
  virtual std::shared_ptr<IUnitTestResultsNode> getResults() = 0;
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
      return spec[r][key]["docs"]["template"].get<std::string>();
    } catch (...) {
      throw INJECTION_EXCEPTION("Template Specification Error: %s:%s/docs does not exist or is not a string ", r.c_str(), key.c_str());
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
      return spec["Introduction"]["docs"]["template"].get<std::string>();
    } catch (...) {
      throw INJECTION_EXCEPTION_("No introduction available");
    }
  }

  std::string conclusion() {
    try {
      return spec["Conclusion"]["docs"]["template"].get<std::string>();
    } catch (...) {
      throw INJECTION_EXCEPTION_("No conclusion available");
    }
  }

  std::string injectionPoint(std::string package, std::string name) const {
    return getter("InjectionPoints", package + ":" + name);
  }

  std::string dataType(std::string key) const { return getter("DataTypes", key); }
  std::string test(std::string package, std::string name) const { return getter("Tests", package + ":" + name); }

  std::string file(std::string package, std::string name) const { return getter("Files", package + ":" + name); }

  std::string action(std::string package, std::string name) const {
    return getter("Actions", package + ":" + name);
  }

  std::string package(std::string package) const { return getter("Options", package); }

  nlohmann::json unitTest(std::string package, std::string name) const {
    return spec["UnitTests"][package + ":" + name];
  }
};


class WalkerNode {
 public:
  std::shared_ptr<Nodes::DataBase> item;
  Nodes::node_type type;
  std::set<long> edges;
};

class WalkerWrapper {
  std::shared_ptr<IWalker> ptr;
  std::shared_ptr<WalkerNode> node;
  IRootNode* _rootNode;
  
  IRootNode* rootNode() {
    return _rootNode; 
  }

 public:
  WalkerWrapper(std::shared_ptr<IWalker> walker, IRootNode* root);
  std::shared_ptr<WalkerNode> next();
};


class IRootNode : public DataBase {
 std::weak_ptr<IRootNode> rootNode_; 
 long idCounter = 0;

 protected:
   virtual void registerNodeInternal(std::shared_ptr<DataBase> ptr) = 0;
   virtual std::shared_ptr<DataBase> findById_Internal(long id) = 0;


 public:
  IRootNode();

  virtual std::shared_ptr<DataBase> findById(long id) {
    if (id == rootNode()->getId()) {
      return rootNode_.lock();
    }
    return findById_Internal(id);
  }


  WalkerWrapper getWalker(std::string package, std::string name, std::string config);

  

  void registerNode(std::shared_ptr<DataBase> ptr) { 
    
    if (ptr->getType() == DataBase::DataType::Root) {
      rootNode_ = ptr->getAsRootNode(ptr);
      ptr->setRootNode(idCounter++, ptr->getAsRootNode(ptr).get());
    } else {
      ptr->setRootNode(idCounter++, rootNode());
    }
    registerNodeInternal(ptr);
  }


  virtual void addIDN(long id, long streamId, node_type type, long index, std::string stage) = 0;
  void join() { auto a = getThread(); if (a!= nullptr) {a->join();} }
  virtual std::thread* getThread() {return NULL;};
  
  virtual std::map<long, std::list<IDN>>& getNodes() = 0; 
  virtual std::shared_ptr<IArrayNode> getChildren() = 0;
  virtual std::shared_ptr<IArrayNode> getUnitTests() = 0;
  virtual std::shared_ptr<IInfoNode> getInfoNode() = 0;
  virtual std::shared_ptr<ICommInfoNode> getCommInfoNode() = 0;
  virtual std::shared_ptr<IMapNode> getPackages() = 0;

  virtual long getEndTime() {
    auto a = getInfoNode();
    if (a != nullptr) {
      auto end = a->getEndTime();
      if ( end <= 0 ) {
       return std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch()
       ).count();
      }
      return end;
    }
    return 0;
  }

  virtual json getDataChildren_(int fileId, int level) override {
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back(getPackages()->getAsDataChild(fileId, level - 1));
    j.push_back(getActions()->getAsDataChild(fileId, level - 1));
    j.push_back(getUnitTests()->getAsDataChild(fileId, level - 1));
    j.push_back(getInfoNode()->getAsDataChild(fileId, level - 1));
    j.push_back(getChildren()->getAsDataChild(fileId, level - 1));

    json jj = json::object();
    jj["text"] = "Injection Points and Log Messages";

    json ch = json::array();
    for (auto it : getNodes()) {
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

  virtual std::shared_ptr<ITestNode> getPackage(std::string package) { 
    auto a = getPackages()->get(package, 0);
    return a->getAsTestNode(a);
  }

  virtual std::shared_ptr<ITestNode> getAction(std::string package) { 
    auto a = getActions()->get(package, 0);
    return a->getAsTestNode(a);
  }

  virtual void lock() = 0;

  virtual std::shared_ptr<IMapNode> getActions() = 0;

  virtual void release() = 0;

  virtual bool hasId(long id) { return findById(id) != NULL; }

  virtual const VnVSpec& getVnVSpec() = 0;

  virtual bool processing() const = 0;

  virtual void respond(long id, long jid, const std::string& response) = 0;

  virtual void persist() {};

  virtual ~IRootNode();
};

}  // namespace Nodes

}  // namespace VnV
#endif  // NODES_H