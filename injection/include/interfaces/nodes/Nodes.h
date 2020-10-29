#ifndef NODES_H
#define NODES_H

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include "base/exceptions.h"
#include "json-schema.hpp"

namespace VnV {

namespace Nodes {

class IArrayNode;
class IDoubleNode;
class IIntegerNode;
class IStringNode;
class ILongNode;
class IFloatNode;
class ILogNode;
class IBoolNode;
class IShapeNode;
class IMapNode;
class IInfoNode;
class ITestNode;
class IInjectionPointNode;
class IUnitTestNode;
class IDataTypeNode;
class IRootNode;

class MetaDataWrapper  {
public:
  std::map<std::string,std::string> m;
  std::string get(std::string key) {
    auto it = m.find(key);
    if (it != m.end()) {
      return it->second;
    }
    throw VnV::VnVExceptionBase("Bad MetaData Key");
  }
  bool has(std::string key) {
     return (m.find(key) != m.end());
  }
  MetaDataWrapper& add(std::string key, std::string value) {
    m.insert(std::make_pair(key,value));
    return *this;
  }

  std::string asJson() {

     nlohmann::json j = m;
     return j.dump();
  }
};


class DataBase {
 public:
  enum class DataType {
    Bool,
    Integer,
    Float,
    Double,
    String,
    Long,
    ShapeArray,
    Array,
    Map,
    Log,
    InjectionPoint,
    Info,
    Test,
    UnitTest,
    DataNode,
    RootNode
  };


  long id;
  DataBase* parent = nullptr;

  MetaDataWrapper metaData;
  MetaDataWrapper& getMetaData() {
    return metaData;
  }

  std::string name;  // name can be assigned, default is to use id (which is unique).
  DataType dataType;
  bool check(DataType type);
  DataBase(DataType type);
  DataType getType();
  IBoolNode* getAsBoolNode();
  IDoubleNode* getAsDoubleNode();
  IIntegerNode* getAsIntegerNode();
  IStringNode* getAsStringNode();
  ILongNode* getAsLongNode();
  IFloatNode* getAsFloatNode();
  IShapeNode* getAsShapeNode();
  IArrayNode* getAsArrayNode();
  ILogNode* getAsLogNode();
  IInjectionPointNode* getAsInjectionPointNode();
  IInfoNode* getAsInfoNode();
  ITestNode* getAsTestNode();
  IDataTypeNode* getAsDataTypeNode();
  IUnitTestNode* getAsUnitTestNode();
  IMapNode* getAsMapNode();
  IRootNode* getAsRootNode();
  long getId();
  std::string getName();

  virtual ~DataBase();
  std::string getTypeStr();

  DataBase* getParent() { return parent; }

  virtual std::string toString() {
    std::ostringstream oss;
    oss << "Node: " << id << "Name: " << name << "DataType: " << getTypeStr();
    return oss.str();
  }
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
  virtual DataBase* get(std::size_t idx) = 0;
  virtual std::size_t size() = 0;
  virtual IArrayNode* add(std::shared_ptr<DataBase> data) = 0;
  virtual ~IArrayNode();

  // Get the value for inserting into a text object.
  virtual std::string getValue() = 0;

  void iter(std::function<void(DataBase*)>& lambda);
};

class IBoolNode : public DataBase {
 public:
  IBoolNode();
  virtual bool getValue() = 0;
  virtual ~IBoolNode();
};

class IDoubleNode : public DataBase {
 public:
  IDoubleNode();
  virtual double getValue() = 0;
  virtual ~IDoubleNode();
};

class IIntegerNode : public DataBase {
 public:
  IIntegerNode();
  virtual int getValue() = 0;
  virtual ~IIntegerNode();
};

class ILongNode : public DataBase {
 public:
  ILongNode();
  virtual long getValue() = 0;
  virtual ~ILongNode();
};

class IStringNode : public DataBase {
 public:
  IStringNode();
  virtual std::string getValue() = 0;
  virtual bool isJson() = 0;
  virtual ~IStringNode();
};

class IFloatNode : public DataBase {
 public:
  IFloatNode();
  virtual float getValue() = 0;
  virtual ~IFloatNode();
};

class IShapeNode : public DataBase {
public:
  IShapeNode();
  virtual ~IShapeNode();
  virtual IArrayNode* getChildren() = 0;
  virtual std::string getShape() = 0;

  virtual DataBase* get(int index);

};

class IInfoNode : public DataBase {
 public:
  IInfoNode();
  virtual std::string getTitle() = 0;
  virtual long getDate() = 0;
  virtual std::string getValue() = 0;
  virtual ~IInfoNode();
};

class ITestNode : public DataBase {
 public:
  ITestNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getData() = 0;
  virtual std::string getValue() = 0;
  virtual IArrayNode* getChildren() = 0;
  virtual ~ITestNode();
};

class IInjectionPointNode : public DataBase {
 public:
  IInjectionPointNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getTests() = 0;
  virtual IArrayNode* getChildren() = 0;
  virtual std::string getValue() = 0;
  virtual ITestNode* getData() = 0;
  virtual std::string getComm() = 0;
  virtual ~IInjectionPointNode();
};

/// TODO -- __internal__ test doesn't get handled very well. In particular, we
/// need to figure out how to access the data for the injection point internal
/// tests and get it inserted into the documentation. getData() -> Array of data
/// elements written during the callback that can be used in the injection point
/// documentation.
///

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
};

class IUnitTestNode : public DataBase {
 public:
  IUnitTestNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getChildren() = 0;
  virtual IMapNode* getResults() = 0;
  virtual std::string getValue() = 0;
  virtual std::string getTestTemplate(std::string name) = 0;
  virtual ~IUnitTestNode();
};

class IDataTypeNode : public DataBase {
 public:
  IDataTypeNode();
  virtual IMapNode* getChildren() = 0;
  virtual long long getDataTypeKey() = 0;
  virtual std::string getValue() = 0;
  virtual ~IDataTypeNode();
};

class IRootNode : public DataBase {
 public:
  IRootNode();
  virtual IArrayNode* getChildren() = 0;
  virtual IArrayNode* getUnitTests() = 0;
  virtual IInfoNode* getInfoNode() = 0;

  virtual std::string getIntro() = 0;
  virtual std::string getConclusion() = 0;

  virtual DataBase* findById(long id) = 0;

  virtual bool hasId(long id) { return findById(id) != NULL; }

  virtual ~IRootNode();
};

}  // namespace Nodes

}  // namespace VnV
#endif  // NODES_H
