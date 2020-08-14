#ifndef OUTPUTREADERIMPLB_H
#define OUTPUTREADERIMPLB_H

#include <iostream>
#include <sstream>
#include <assert.h>

#include "c-interfaces/PackageName.h"
#include "interfaces/nodes/Nodes.h"
#include "json-schema.hpp"

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace JsonReader {

using namespace VnV::Nodes;

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
  virtual DataBase* get(std::size_t idx) override;
  virtual std::size_t size() override;
  virtual IArrayNode* add(std::shared_ptr<DataBase> data) override;

  std::string getValue() override { return templ; }
};

class BoolNode : public IBoolNode {
 public:
  bool value;
  BoolNode();
  virtual bool getValue() override;
};

class ShapeNode : public IShapeNode {
public:
   std::shared_ptr<ArrayNode> children;
   nlohmann::json shape;

   IArrayNode* getChildren() override;
   std::string getShape() override;

};

class DoubleNode : public IDoubleNode {
 public:
  double value;
  DoubleNode();
  virtual double getValue() override;
};

class IntegerNode : public IIntegerNode {
 public:
  IntegerNode();
  int value;
  virtual int getValue() override;
};

class LongNode : public ILongNode {
 public:
  LongNode();
  long value;
  virtual long getValue() override;
};

class StringNode : public IStringNode {
 public:
  StringNode();
  std::string value;
  bool jsonString = false;
  virtual std::string getValue() override;
  virtual bool isJson() override;
};

class FloatNode : public IFloatNode {
 public:
  FloatNode();
  float value;
  virtual float getValue() override;
};

class InfoNode : public IInfoNode {
 public:
  InfoNode();
  std::string title, templ;
  long date;
  virtual std::string getTitle() override;

  virtual long getDate() override;

  virtual std::string getValue() override { return templ; }
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

  InjectionPointNode();
  virtual std::string getPackage() override;
  virtual IArrayNode* getTests() override;
  virtual ITestNode* getData() override { return internal.get(); }

  virtual std::string getValue() override { return templ; }
  virtual IArrayNode* getChildren() override;
};

class LogNode : public ILogNode {
 public:
  static std::string def;
  std::string package, level, stage, message, templ;
  LogNode();
  virtual std::string getPackage() override;
  virtual std::string getLevel() override;
  virtual std::string getMessage() override;
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

class DataTypeNode : public IDataTypeNode {
 public:
  long long key;
  std::string package, templ;
  std::shared_ptr<MapNode> children;
  DataTypeNode();
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
  virtual IArrayNode* getChildren() override;
  virtual IArrayNode* getUnitTests() override;
  virtual IInfoNode* getInfoNode() override;

  virtual DataBase* findById(long id) override;

  void add(std::shared_ptr<DataBase> ptr) { idMap[ptr->getId()] = ptr; }

  virtual std::string getConclusion() override { return concl; }

  virtual std::string getIntro() override { return intro; }
};

// Debug for now
IRootNode* parse(std::string filename, long& idCounter);

}  // namespace JsonReader
}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV

#endif  // OUTPUTREADERIMPL_H
