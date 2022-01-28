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

#include "streaming/Nodes.h"
#include "streaming/common.h"
#include "streaming/streamtemplate.h"

namespace VnV {
namespace Nodes {
namespace Memory {

#define INITMEMBER(name, Type)      \
  if (name == nullptr) {            \
    name.reset(new Type());         \
    rootNode()->registerNode(name); \
    name->setname(#name);           \
  }

class InMemory {
 public:
  template <typename T> class DataBaseImpl : public T {
   private:
    MetaDataWrapper metadata;
    bool open_;
    long streamId;
    std::string name;

   public:
    DataBaseImpl() : T() {}

    GETTERSETTER(streamId, long);
    GETTERSETTER(name, std::string);
    GETTERSETTER(metadata, MetaDataWrapper);

    virtual const MetaDataWrapper& getMetaData() override { return metadata; };
    virtual std::string getNameInternal() override { return name; }
    virtual bool getopen() override { return open_; };
    virtual void setopen(bool value) override { open_ = value; }
    virtual long getStreamId() override { return streamId; }
  };

  class ArrayNode : public DataBaseImpl<IArrayNode> {
    std::vector<std::shared_ptr<DataBase>> vec;

   public:
    ArrayNode() : DataBaseImpl<IArrayNode>() {}

    virtual std::shared_ptr<DataBase> get(std::size_t idx) override {
      return (idx < vec.size()) ? (vec[idx]) : nullptr;
    }

    virtual std::size_t size() override { return vec.size(); };

    virtual void add(std::shared_ptr<DataBase> data) override { vec.push_back(data); }
  };

  class MapNode : public DataBaseImpl<IMapNode> {
    std::map<std::string, std::shared_ptr<ArrayNode>> map;

   public:
    MapNode() : DataBaseImpl<IMapNode>() {}

    virtual void insert(std::string key, std::shared_ptr<DataBase> val) {
      auto it = map.find(key);
      if (it != map.end()) {
        it->second->add(val);
      } else {
        auto a = std::make_shared<ArrayNode>();
        rootNode()->registerNode(a);
        a->setname(key);
        a->add(val);
        map[key] = a;
      }
    }

    virtual std::shared_ptr<IArrayNode> get(std::string key) override {
      auto it = map.find(key);
      return (it == map.end()) ? nullptr : (it->second);
    }

    virtual bool contains(std::string key) override { return map.find(key) != map.end(); }

    virtual std::vector<std::string> fetchkeys() override {
      std::vector<std::string> v;
      for (auto it = map.begin(); it != map.end(); ++it) {
        v.push_back(it->first);
      }
      return v;
    }

    virtual std::size_t size() override { return map.size(); };

    virtual ~MapNode(){};
  };

#define X(x, y)                                                                                         \
  class x##Node : public DataBaseImpl<I##x##Node> {                                                     \
    std::vector<std::size_t> shape;                                                                     \
    std::vector<y> value;                                                                               \
                                                                                                        \
   public:                                                                                              \
    x##Node() : DataBaseImpl<I##x##Node>() {}                                                           \
                                                                                                        \
    const std::vector<std::size_t>& getShape() override { return shape; }                               \
                                                                                                        \
    y getValueByShape(const std::vector<std::size_t>& rshape) override {                                \
      if (shape.size() == 0) {                                                                          \
        return value[0];                                                                                \
      }                                                                                                 \
      if (rshape.size() != shape.size())                                                                \
        throw INJECTION_EXCEPTION("%s: Invalid Shape Size %d (should be %d)", #x,  rshape.size(), shape.size()); \
      int index = 0;                                                                                    \
      for (int i = 0; i < shape.size(); i++) {                                                          \
        index += rshape[i] * shape[i];                                                                  \
      }                                                                                                 \
      return getValueByIndex(index);                                                                    \
    }                                                                                                   \
    void add(const y& v) { value.push_back(v); }                                                        \
    y getValueByIndex(const size_t ind) override { return value[ind]; }                                 \
                                                                                                        \
    y getScalarValue() override {                                                                       \
      if (shape.size() == 0)                                                                            \
        return value[0];                                                                                \
      else                                                                                              \
        throw INJECTION_EXCEPTION("%s: No shape provided to non scalar shape tensor object", #x);       \
    }                                                                                                   \
                                                                                                        \
    int getNumElements() override { return value.size(); }                                              \
    virtual ~x##Node() {}                                                                               \
    virtual void setShape(const std::vector<std::size_t>& s) { shape = s; }                             \
    virtual void setValue(const std::vector<y>& s) { value = s; }                                       \
  };
  DTYPES
#undef X

  class InfoNode : public DataBaseImpl<IInfoNode> {
    long start = 0;
    long end = 0;
    std::string title;
    std::string workflow;
    std::string jobName;
    std::shared_ptr<VnVProv> prov = nullptr;

    virtual std::shared_ptr<VnVProv> getProvInternal() { return prov; };

   public:
    InfoNode() : DataBaseImpl<IInfoNode>() {}
    virtual std::string getTitle() override { return title; }
    virtual long getStartTime() override { return start; }
    virtual long getEndTime() override { return end; }
    virtual std::string getWorkflow() override { return workflow; }
    virtual std::string getJobName() override { return jobName; }
    
    GETTERSETTER(workflow,std::string)
    GETTERSETTER(jobName,std::string)
    GETTERSETTER(start, long)
    GETTERSETTER(end,long)
    GETTERSETTER(title, std::string)
    GETTERSETTER(prov, std::shared_ptr<VnVProv>)

    virtual ~InfoNode() {}
  };

  class CommInfoNode : public DataBaseImpl<ICommInfoNode> {
    std::shared_ptr<CommMap> commMap;
    int worldSize;
    json nodeMap;
    std::string version;

   protected:
    virtual std::shared_ptr<ICommMap> getCommMapInternal() override { return commMap; }

   public:
    CommInfoNode() : DataBaseImpl<ICommInfoNode>(), commMap(new CommMap()) {}

    virtual int getWorldSize() override { return worldSize; }
    virtual std::string getNodeMap() override { return nodeMap.dump(); }
    virtual std::string getVersion() override { return version; }

    GETTERSETTER(worldSize, int);
    GETTERSETTER(nodeMap, json);
    GETTERSETTER(version, std::string);

    virtual ~CommInfoNode(){};
  };

  class WorkflowNode : public DataBaseImpl<IWorkflowNode> {
    std::string package = "";
    std::string state = "";
    json info;
    std::map<std::string, std::shared_ptr<IRootNode>> rootNodes;
    std::map<std::string, int> fileIds;
  
  public:
  
    WorkflowNode() : DataBaseImpl<IWorkflowNode>() {}
    virtual std::string getPackage() override { return package; }
    virtual std::string getState() override { return state; }
    virtual json getInfo() override { return info; }
    
    GETTERSETTER(package, std::string);
    GETTERSETTER(state, std::string);
    GETTERSETTER(info, json);

    virtual std::shared_ptr<IRootNode> getReport(std::string reportName) override {
     
     auto it = rootNodes.find(reportName);
     if (it != rootNodes.end()) {
       return it->second;
     }
     return nullptr;
   }
 
   virtual bool hasReport(std::string reportName) override {
     return rootNodes.find( reportName) != rootNodes.end();
   }

   virtual void setReport(std::string reportName, int fileId, std::shared_ptr<IRootNode> rootNode) override {
     fileIds[ reportName ] = fileId ;
     rootNodes[ reportName ] = rootNode; 
   }

   virtual std::vector<std::string> listReports() override {
     std::vector<std::string> ret; 
     for (auto &it : rootNodes) {
       ret.push_back(it.first);
     }
     return ret;
   }

   virtual int getReportFileId(std::string reportName) {
      auto it = fileIds.find(reportName);
     if (it != fileIds.end()) {
       return it->second;
     }
     return -100;
   }

  };

  class TestNode : public DataBaseImpl<ITestNode> {
    long uid;
    ITestNode::TestNodeUsage usage;
    std::string package;
    std::shared_ptr<MapNode> data = nullptr;
    std::shared_ptr<ArrayNode> logs = nullptr;
    bool result;
    bool internal;

   public:
    TestNode() : DataBaseImpl<ITestNode>() {}
    virtual std::string getPackage() override { return package; }
    virtual std::shared_ptr<IMapNode> getData() override {
      INITMEMBER(data, MapNode)
      return data;
    }
    virtual std::shared_ptr<IArrayNode> getLogs() override {
      INITMEMBER(logs, ArrayNode)
      return logs;
    }

    virtual ITestNode::TestNodeUsage getUsage() override { return usage; }
    virtual bool isInternal() override { return internal; }

    GETTERSETTER(uid, long);
    GETTERSETTER(result, bool);
    GETTERSETTER(internal, bool);
    GETTERSETTER(usage, ITestNode::TestNodeUsage);
    GETTERSETTER(package, std::string);
    GETTERSETTER(data, std::shared_ptr<MapNode>)
    GETTERSETTER(logs, std::shared_ptr<ArrayNode>)
  };

  class InjectionPointNode : public DataBaseImpl<IInjectionPointNode> {
    std::shared_ptr<ArrayNode> logs;
    std::shared_ptr<ArrayNode> tests;
    std::shared_ptr<TestNode> internal;
    std::string package;

    nlohmann::json sourceMap = json::object();

    long startIndex = -1;
    long endIndex = -1;
    long startTime = -1;
    long endTime = -1;
    long long commId;

    bool isIter = false;  // internal property to help with parsing.
    bool isOpen = false;  // internal property to help with paresing

   public:
    GETTERSETTER(package, std::string)
    GETTERSETTER(sourceMap, nlohmann::json)
    GETTERSETTER(commId, long long)
    GETTERSETTER(internal, std::shared_ptr<TestNode>)
    GETTERSETTER(startTime, long);
    GETTERSETTER(endTime, long);
    GETTERSETTER(startIndex, long)
    GETTERSETTER(endIndex, long)
    GETTERSETTER(isIter, bool)
    GETTERSETTER(isOpen, bool)

    void addToSourceMap(std::string stage, std::string function, int line) {
      if (!sourceMap.contains(stage)) {
        json j = json::array();
        j.push_back(function);
        j.push_back(line);
        sourceMap[stage] = j;
      }
    }

    InjectionPointNode() : DataBaseImpl<IInjectionPointNode>() {}

    virtual std::string getPackage() override { return package; }

    virtual std::shared_ptr<IArrayNode> getTests() override {
      INITMEMBER(tests, ArrayNode)
      return tests;
    }

    virtual std::shared_ptr<ITestNode> getData() override {
      INITMEMBER(internal, TestNode)
      return internal;
    }

    virtual std::shared_ptr<IArrayNode> getLogs() override {
      INITMEMBER(logs, ArrayNode);
      return logs;
    }

    virtual std::string getComm() override { return std::to_string(commId); }
    virtual std::string getSourceMap() override { return sourceMap.dump(); }

    virtual long getStartIndex() override { return startIndex; }
    virtual long getEndIndex() override { return endIndex; }
    virtual long getStartTime() override { return startTime; }
    virtual long getEndTime() override { return endTime; }
    std::shared_ptr<TestNode> getTestByUID(long uid) {
      for (int i = 0; i < getTests()->size(); i++) {
        auto t = std::dynamic_pointer_cast<TestNode>(getTests()->get(i));
        if (t->getuid() == uid) {
          return t;
        }
      }
      return nullptr;
    }

    virtual ~InjectionPointNode() {}
  };

  class LogNode : public DataBaseImpl<ILogNode> {
    std::string package, level, stage, message, comm;
    int identity;

   public:
    GETTERSETTER(package, std::string)
    GETTERSETTER(level, std::string)
    GETTERSETTER(stage, std::string)
    GETTERSETTER(message, std::string)
    GETTERSETTER(comm, std::string)
    GETTERSETTER(identity, int)

    LogNode() : DataBaseImpl<ILogNode>() {}
    virtual std::string getPackage() override { return package; }
    virtual std::string getLevel() override { return level; }
    virtual std::string getMessage() override { return message; }
    virtual std::string getComm() override { return comm; }
    virtual std::string getStage() override { return stage; }
    virtual ~LogNode() {}
  };

  class DataNode : public DataBaseImpl<IDataNode> {
    bool local;
    long long key;
    std::shared_ptr<ArrayNode> logs;
    std::string package;
    std::shared_ptr<MapNode> children;

   public:
    GETTERSETTER(local, bool)
    GETTERSETTER(key, long long)
    GETTERSETTER(package, std::string)

    DataNode() : DataBaseImpl<IDataNode>() {}

    virtual bool getLocal() override { return local; }
    virtual long long getDataTypeKey() override { return key; }

    virtual std::shared_ptr<IMapNode> getData() override {
      INITMEMBER(children, MapNode)
      return children;
    }

    virtual std::shared_ptr<IArrayNode> getLogs() override {
      INITMEMBER(logs, ArrayNode);
      return logs;
    };

    virtual ~DataNode() {}
  };

  class UnitTestResultNode : public DataBaseImpl<IUnitTestResultNode> {
    std::string desc;
    bool result;

   public:
    GETTERSETTER(desc, std::string)
    GETTERSETTER(result, bool)

    UnitTestResultNode() : DataBaseImpl<IUnitTestResultNode>() {}
    virtual std::string getDescription() override { return desc; }
    virtual bool getResult() override { return result; }
    virtual ~UnitTestResultNode() {}
  };

  class UnitTestResultsNode : public DataBaseImpl<IUnitTestResultsNode> {
    std::shared_ptr<MapNode> m;

    auto getM() {
      INITMEMBER(m, MapNode)
      return m;
    }

   public:
    UnitTestResultsNode() : DataBaseImpl<IUnitTestResultsNode>() {}

    virtual std::shared_ptr<IUnitTestResultNode> get(std::string key) {
      if (m->contains(key)) {
        auto a = getM()->get(key);
        auto b = a->getAsArrayNode(a)->get(0);
        return a->getAsUnitTestResultNode(a);
      }
      throw INJECTION_EXCEPTION("Unit Test Results Node: Key %s does not exist", key.c_str());
    };

    void insert(std::string name, std::shared_ptr<IUnitTestResultNode> value) { getM()->insert(name, value); }

    virtual bool contains(std::string key) { return getM()->contains(key); }

    virtual std::vector<std::string> fetchkeys() { return getM()->fetchkeys(); };

    virtual ~UnitTestResultsNode(){};
  };

  class UnitTestNode : public DataBaseImpl<IUnitTestNode> {
    std::string package;
    std::shared_ptr<ArrayNode> logs;
    std::shared_ptr<MapNode> children;
    std::shared_ptr<UnitTestResultsNode> resultsMap;
    std::map<std::string, std::string> testTemplate;

   public:
    GETTERSETTER(package, std::string)

    UnitTestNode() : DataBaseImpl<IUnitTestNode>() {}

    virtual std::string getPackage() override { return package; }

    virtual std::shared_ptr<IMapNode> getData() override {
      INITMEMBER(children, MapNode);
      return children;
    }

    virtual std::shared_ptr<IArrayNode> getLogs() override {
      INITMEMBER(logs, ArrayNode)
      return logs;
    };

    virtual std::shared_ptr<IUnitTestResultsNode> getResults() override {
      INITMEMBER(resultsMap, UnitTestResultsNode)
      return resultsMap;
    }
  };

  class RootNode : public DataBaseImpl<IRootNode> {
    long lowerId, upperId;
    std::atomic<bool> _processing = ATOMIC_VAR_INIT(true);

    std::shared_ptr<VnVSpec> spec;
    std::shared_ptr<ArrayNode> children;
    std::shared_ptr<ArrayNode> unitTests;
    std::shared_ptr<MapNode> actions;
    std::shared_ptr<MapNode> packages;
    std::shared_ptr<InfoNode> infoNode;
    std::shared_ptr<CommInfoNode> commInfo;
    std::shared_ptr<WorkflowNode> workflowNode;


    std::map<long, std::list<IDN>> nodes;
    std::map<long, std::shared_ptr<DataBase>> idMap;

   public:
    GETTERSETTER(lowerId, long);
    GETTERSETTER(upperId, long);
    GETTERSETTER(infoNode, std::shared_ptr<InfoNode>)
    GETTERSETTER(workflowNode, std::shared_ptr<WorkflowNode>)

    RootNode() : DataBaseImpl<IRootNode>(), spec(new VnVSpec()) {}

    void setspec(const json& s) { spec->set(s); }

    void setProcessing(bool value) { _processing.store(value, std::memory_order_relaxed); }

    virtual std::shared_ptr<IMapNode> getPackages() override { INITMEMBER(packages, MapNode) return packages; }
    virtual std::shared_ptr<IMapNode> getActions() override { INITMEMBER(actions, MapNode) return actions; }
    virtual std::shared_ptr<IArrayNode> getChildren() override { INITMEMBER(children, ArrayNode) return children; }
    virtual std::shared_ptr<IArrayNode> getUnitTests() override { INITMEMBER(unitTests, ArrayNode) return unitTests; }
    virtual std::shared_ptr<IInfoNode> getInfoNode() override { INITMEMBER(infoNode, InfoNode) return infoNode; }
    virtual std::shared_ptr<IWorkflowNode> getWorkflowNode() override { INITMEMBER(workflowNode, WorkflowNode) return workflowNode; }
    virtual std::shared_ptr<ICommInfoNode> getCommInfoNode() override {
      INITMEMBER(commInfo, CommInfoNode) return commInfo;
    }

    virtual bool processing() const override { return _processing.load(); }

    virtual std::shared_ptr<DataBase> findById_Internal(long id) override {
      auto it = idMap.find(id);
      if (it != idMap.end()) {
        return it->second;
      }
      throw INJECTION_EXCEPTION("Invalid Id %s", id);
    }

    virtual void registerNodeInternal(std::shared_ptr<DataBase> ptr) { idMap[ptr->getId()] = ptr; }

    virtual std::map<long, std::list<IDN>>& getNodes() override { return nodes; }

    void addIDN(long id, long streamId, node_type type, long index, std::string stage) override {
      auto it = nodes.find(index);
      if (it == nodes.end()) {
        nodes[index] = std::list<IDN>();
        nodes[index].push_back(IDN(id, streamId, type, stage));
      } else {
        (it->second).push_back(IDN(id, streamId, type, stage));
      }
    }

    virtual const VnVSpec& getVnVSpec() { return *spec; }
  };
};


typedef StreamParserTemplate<InMemory> InMemoryParser;
template <typename N> using InMemoryParserVisitor = InMemoryParser::ParserVisitor<N>;
template <typename N> using NoLockInMemoryParserVisitor = InMemoryParser::NoLockParserVisitor<N>;
template <typename T, typename V> using InMemoryRootNodeWithThread = InMemoryParser::RootNodeWithThread<T, V>;

}  // namespace Memory
}  // namespace Nodes
}  // namespace VnV

#endif
