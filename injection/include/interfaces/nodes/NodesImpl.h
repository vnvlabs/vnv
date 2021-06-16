#ifndef NODESIMPLB_H
#define NODESIMPLB_H

#include <assert.h>

#include <iostream>
#include <sstream>

#include "c-interfaces/PackageName.h"
#include "interfaces/nodes/Nodes.h"
#include "json-schema.hpp"

namespace VnV {
namespace Nodes {

class MapNode : public IMapNode {
 public:
  std::string templ;

  std::map<std::string, std::shared_ptr<DataBase>> value;

  MapNode();
  virtual DataBase* get(std::string key) override;
  virtual IMapNode* add(std::string key, std::shared_ptr<DataBase> v) override;
  virtual bool contains(std::string key) override;
  virtual std::vector<std::string> fetchkeys() override;

  virtual std::string getValue() override { return templ; }

  virtual std::size_t size() override;
};

class ArrayNode : public IArrayNode {
 public:
  std::string templ;

  ArrayNode();
  std::vector<std::shared_ptr<DataBase>> value;
  virtual std::shared_ptr<DataBase> getShared(std::size_t idx) override;
  virtual std::size_t size() override;
  virtual IArrayNode* add(std::shared_ptr<DataBase> data) override;

  std::string getValue() override { return templ; }
};

#define X(x, y)                                                         \
  class x##Node : public I##x##Node {                                   \
   public:                                                              \
    std::vector<std::size_t> shape;                                     \
    std::vector<y> value;                                               \
    const std::vector<std::size_t>& getShape() override;                \
    x##Node();                                                          \
    y getValue(const std::vector<std::size_t>& rshape) override; \
    y getValue(const size_t ind) override;                       \
    virtual ~x##Node();                                                 \
  };
DTYPES
#undef X

class InfoNode : public IInfoNode {
 public:
  InfoNode();
  std::string title, templ;
  long date;
  virtual std::string getTitle() override;
  virtual long getDate() override;
  virtual std::string getValue() override { return templ; }
};

class CommInfoNode : public ICommInfoNode {
 public:
  CommInfoNode();
  nlohmann::json commMap;
  int worldSize;

  virtual int getWorldSize() override;
  virtual std::string getCommMap() override;
  virtual ~CommInfoNode(){};
};

class TestNode : public ITestNode {
 public:
  std::string package, templ;
  std::shared_ptr<ArrayNode> data;
  std::shared_ptr<ArrayNode> children;
  TestNode();
  virtual std::string getPackage() override;
  virtual IArrayNode* getData() override;
  virtual IArrayNode* getChildren() override;
  virtual std::string getValue() override { return templ; }
};

class InjectionPointNode : public IInjectionPointNode {
 public:
  std::shared_ptr<ArrayNode> children;
  std::shared_ptr<ArrayNode> tests;
  std::shared_ptr<TestNode> internal;
  std::string package;
  std::string templ;

  std::pair<int,int> identity;
  long long commId;

  InjectionPointNode();
  virtual std::string getPackage() override;
  virtual IArrayNode* getTests() override;
  virtual ITestNode* getData() override { return internal.get(); }
  virtual std::string getComm() override { return std::to_string(commId); }
  virtual std::string getValue() override { return templ; }
  virtual IArrayNode* getChildren() override;
};

class LogNode : public ILogNode {
 public:
  static std::string def;
  std::string package, level, stage, message, templ, comm;

  int identity;

  LogNode();
  virtual std::string getPackage() override;
  virtual std::string getLevel() override;
  virtual std::string getMessage() override;
  virtual std::string getComm() override { return comm; }
  virtual std::string getValue() override {
    if (!templ.empty()) return templ;
    if (level == "Warning") {
      return ".. caution::\n    " + def;
    } else if (level == "Error") {
      return ".. error::\n    " + def;
    } else if (level == "Info") {
      return ".. important::\n    " + def;
    }
    return ".. note::\n    " + def;
  }

  virtual std::string getStage() override;
};

class DataNode : public IDataNode {
 public:
  bool local;
  long long key;
  std::string package, templ;
  std::shared_ptr<MapNode> children;
  DataNode();
  virtual bool getLocal() override {return local;}
  virtual long long getDataTypeKey() override;
  virtual IMapNode* getChildren() override;
  virtual std::string getValue() override { return templ; }
};

class UnitTestNode : public IUnitTestNode {
 public:
  std::string package, templ;
  std::shared_ptr<ArrayNode> children;
  std::shared_ptr<MapNode> resultsMap;
  std::map<std::string, std::string> testTemplate;

  UnitTestNode();
  virtual std::string getPackage() override;
  virtual IArrayNode* getChildren() override;
  virtual std::string getValue() override { return templ; }
  virtual std::string getTestTemplate(std::string name) override {
    auto it = testTemplate.find(name);
    return (it == testTemplate.end()) ? "" : it->second;
  }

  virtual IMapNode* getResults() override;
};

class RootNode : public IRootNode {
 public:
  std::string intro, concl;
  std::map<long, std::shared_ptr<DataBase>> idMap;
  long lowerId, upperId;

  std::shared_ptr<ArrayNode> children;
  std::shared_ptr<ArrayNode> unitTests;
  std::shared_ptr<InfoNode> infoNode;
  std::shared_ptr<ICommInfoNode> commInfo;

  virtual std::string getCommMap() override { return commInfo->getCommMap(); }
  virtual int getWorldSize() override { return commInfo->getWorldSize(); }

  virtual IArrayNode* getChildren() override;
  virtual IArrayNode* getUnitTests() override;
  virtual IInfoNode* getInfoNode() override;
  virtual ICommInfoNode* getCommInfoNode() override;

  virtual DataBase* findById(long id) override;

  void add(std::shared_ptr<DataBase> ptr) { idMap[ptr->getId()] = ptr; }

  virtual std::string getConclusion() override { return concl; }

  virtual std::string getIntro() override { return intro; }
};


class SharedRootNodeWrapper : public IRootNode {

  std::shared_ptr<RootNode> root;
 public:

  SharedRootNodeWrapper(std::shared_ptr<RootNode> wrapper ) : root(wrapper){
  }

  virtual std::string getCommMap() override { return root->getCommMap(); }
  virtual int getWorldSize() override { return root->getWorldSize(); }

  virtual IArrayNode* getChildren() override {return root->getChildren();}
  virtual IArrayNode* getUnitTests() override {return root->getUnitTests();}
  virtual IInfoNode* getInfoNode() override {return root->getInfoNode();}
  virtual ICommInfoNode* getCommInfoNode() override {return root->getCommInfoNode();}

  virtual DataBase* findById(long id) override {return root->findById(id);}

  virtual std::string getConclusion() override { return root->getConclusion(); }

  virtual std::string getIntro() override { return root->getIntro(); }

};


}  // namespace Nodes
}  // namespace VnV

#endif  // OUTPUTREADERIMPL_H
