#ifndef NODES_H
#define NODES_H

#include <assert.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include <list>

#include "base/exceptions.h"
#include "json-schema.hpp"

namespace VnV {

class IWalker;
class WalkerNode;

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
  bool open = false;
  long id;
  long streamId;

  MetaDataWrapper metaData;
  std::string
      name;  // name can be assigned, default is to use id (which is unique).
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

  virtual long getStreamId() { return streamId; }

  virtual std::string toString() {
    std::ostringstream oss;
    oss << "Node: " << id << "Name: " << name << "DataType: " << getTypeStr();
    return oss.str();
  }
};

class DataBaseWithChildren {
 public:
  virtual IArrayNode* getChildren() = 0;
};

#define X(X, x)                                                           \
  class I##X##Node : public DataBase {                                    \
   public:                                                                \
    I##X##Node();                                                         \
    virtual const std::vector<std::size_t>& getShape() = 0;               \
    virtual x getValueByShape(const std::vector<std::size_t>& shape) = 0; \
    virtual x getValueByIndex(const std::size_t ind) = 0;                 \
    virtual std::vector<x>& getRawVector() = 0;                           \
    virtual x getScalarValue() = 0;                                       \
    virtual int getNumElements() = 0;                                     \
    virtual ~I##X##Node();                                                \
    virtual std::string getShapeJson() {                                  \
      nlohmann::json j = this->getShape();                                \
      return j.dump();                                                    \
    }                                                                     \
  };
DTYPES
#undef X

class IDataNode : public DataBase {
 public:
  IDataNode();
  virtual IArrayNode* getLogs() = 0;
  virtual IMapNode* getChildren() = 0;
  virtual long long getDataTypeKey() = 0;
  virtual std::string getValue() = 0;
  virtual bool getLocal() = 0;
  virtual ~IDataNode();
};

class IMapNode : public DataBase {
 public:
  IMapNode();
  virtual DataBase* get(std::string key) = 0;
  virtual IMapNode* add(std::string key, std::shared_ptr<DataBase> v) = 0;
  virtual bool contains(std::string key) = 0;
  virtual std::vector<std::string> fetchkeys() = 0;
  virtual std::size_t size() = 0;

  virtual std::string getValue() = 0;
  virtual ~IMapNode();
};

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
};

class IInfoNode : public DataBase {
 public:
  IInfoNode();
  virtual std::string getTitle() = 0;
  virtual long getDate() = 0;
  virtual std::string getValue() = 0;
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
};

class ICommInfoNode : public DataBase {
 public:
  ICommInfoNode();
  virtual int getWorldSize() = 0;
  virtual ICommMap* getCommMap() = 0;
  virtual ~ICommInfoNode();
};

class ITestNode : public DataBase {
 public:
  ITestNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getData() = 0;
  virtual std::string getValue() = 0;
  virtual IArrayNode* getLogs() = 0;
  virtual ~ITestNode();
};

class IInjectionPointNode : public DataBase {
 public:
  IInjectionPointNode();
  virtual long getStartIndex() = 0;
  virtual long getEndIndex() = 0;
  virtual std::string getComm() = 0;
  virtual double getTime() = 0;
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getTests() = 0;
  virtual std::string getValue() = 0;
  virtual IArrayNode* getLogs() = 0;
  virtual ITestNode* getData() = 0;
  virtual std::string getTimes() = 0;

  virtual ~IInjectionPointNode();
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
  virtual double getTime() = 0;
  virtual ~ILogNode();
};

class IUnitTestResultNode : public DataBase {
 public:
  IUnitTestResultNode();
  virtual std::string getTemplate() = 0;
  virtual std::string getDescription() = 0;
  virtual bool getResult() = 0;
  virtual ~IUnitTestResultNode();
};

class IUnitTestResultsNode : public DataBase {
 public:
  IUnitTestResultsNode();
  virtual IUnitTestResultNode* get(std::string key) = 0;
  virtual bool contains(std::string key) = 0;
  virtual std::vector<std::string> fetchkeys() = 0;
  virtual ~IUnitTestResultsNode();
};

class IUnitTestNode : public DataBase, public DataBaseWithChildren {
 public:
  IUnitTestNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getChildren() = 0;
  virtual IArrayNode* getLogs() = 0;
  virtual IUnitTestResultsNode* getResults() = 0;
  virtual std::string getValue() = 0;
  virtual std::string getTestTemplate(std::string name) = 0;
  virtual ~IUnitTestNode();
};

class VnVSpec {
  nlohmann::json spec;

  std::string getter(std::string r, std::string key) const {
    return spec[r][key]["docs"].get<std::string>();
  }

 public:
  VnVSpec(const nlohmann::json& j) : spec(j) {}

  template <typename T>
  VnVSpec(const T& j) : spec(nlohmann::json::parse(j.dump())){};

  VnVSpec() {}
  void set(const nlohmann::json& s) { spec = s; }

  std::string get() { return spec.dump(); }

  std::string intro() {
    return spec["Introduction"]["docs"].get<std::string>();
  }

  std::string conclusion() {
    return spec["Conclusion"]["docs"].get<std::string>();
  }

  std::string injectionPoint(std::string package, std::string name) const {
    return getter("InjectionPoints", package + ":" + name);
  }

  std::string dataType(std::string key) const {
    return getter("DataTypes", key);
  }
  std::string test(std::string package, std::string name) const {
    return getter("Tests", package + ":" + name);
  }

  nlohmann::json unitTest(std::string package, std::string name) const {
    return spec["UnitTests"][package + ":" + name];
  }
};

enum class node_type { ROOT, POINT, START, ITER, END, LOG, DONE };

class WalkerWrapper {
  std::shared_ptr<IWalker> ptr;
  std::shared_ptr<WalkerNode> node;
 public:
  WalkerWrapper(std::shared_ptr<IWalker> walker);
  
  WalkerNode* next();

};

class IRootNode : public DataBase, public DataBaseWithChildren {
  // index -> streamId -> nodeId
  std::map<long, std::list<std::tuple<long, long, node_type>>> nodes;
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
    throw VnV::VnVExceptionBase("Invalid Id");
  }

  void registerWalkerCallback(IWalker* walker) {
    walkers.insert(walker);
  }
  void deregisterWalkerCallback(IWalker* walker) {
    walkers.erase(walker);
  }

  virtual void add(std::shared_ptr<DataBase> ptr) { idMap[ptr->getId()] = ptr; }

  virtual void addIDN(long id, long streamId, node_type type, long index);

  virtual IArrayNode* getChildren() = 0;
  virtual IArrayNode* getUnitTests() = 0;
  virtual IInfoNode* getInfoNode() = 0;
  virtual ICommInfoNode* getCommInfoNode() = 0;

  virtual bool hasId(long id) { return findById(id) != NULL; }

  virtual const VnVSpec& getVnVSpec() = 0;

  virtual std::map<long, std::list<std::tuple<long, long, node_type>>>&
  getNodes() {
    return nodes;
  }

  WalkerWrapper getWalker(std::string package, std::string name,
                          std::string config);

  virtual ~IRootNode();
};

}  // namespace Nodes

}  // namespace VnV
#endif  // NODES_H
