#ifndef NODES_H
#define NODES_H

#include <assert.h>

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "base/exceptions.h"
#include "json-schema.hpp"

namespace VnV {

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


#define DTYPES X(Bool,bool) X(Integer,int) X(Float,float) X(Double,double) X(String,std::string) X(Json, std::string) X(Long,long) X(Shape,std::shared_ptr<DataBase>)
#define STYPES X(Array) X(Map) X(Log) X(InjectionPoint) X(Info) X(CommInfo) X(Test) X(UnitTest) X(Data)
#define RTYPES X(Root)

#define X(x,y) class I##x##Node;
DTYPES
#undef X
#define X(x) class I##x##Node;
STYPES
RTYPES
#undef X


class DataBase {
 public:

  enum class DataType {
    #define X(x,y) x,
      DTYPES
    #undef X

    #define X(x) x,
      STYPES
    #undef X

    Root
  };
  bool open = false;

  long id;
  std::vector<std::shared_ptr<DataBase>> parent;

  MetaDataWrapper metaData;
  MetaDataWrapper& getMetaData() { return metaData; }

  std::string
      name;  // name can be assigned, default is to use id (which is unique).
  DataType dataType;
  bool check(DataType type);
  DataBase(DataType type);

  DataType getType();

#define X(x,y) I##x##Node* getAs##x##Node();
  DTYPES
#undef X
#define X(x) I##x##Node* getAs##x##Node();
  STYPES
  RTYPES
#undef X

  long getId();
  std::string getName();

  virtual ~DataBase();
  std::string getTypeStr();

  std::vector<DataBase*> getParents() {
    std::vector<DataBase*> db(parent.size());
    for (auto it : parent) {
      db.push_back(it.get());
    }
    return db;
  }

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

#define X(X,x)                                          \
  class I##X##Node : public DataBase {                             \
   public:                                                         \
    I##X##Node();                                                  \
    virtual const std::vector<std::size_t>& getShape() = 0;               \
    virtual x getValue(const std::vector<std::size_t>& shape) = 0; \
    virtual x getValue(const std::size_t ind) = 0;           \
    virtual ~I##X##Node();                                         \
    virtual std::string getShapeJson() { \
       nlohmann::json j = this->getShape(); \
       return j.dump(); \
    }\
  };
DTYPES
#undef X


class IDataNode : public DataBase {
 public:
  IDataNode();
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
  virtual DataBase* get(std::size_t idx){
      return getShared(idx).get();
  };
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

class ICommInfoNode : public DataBase {
 public:
   ICommInfoNode();
   virtual int getWorldSize() = 0;
   virtual std::string getCommMap() = 0;
   virtual ~ICommInfoNode();
};

class ITestNode : public DataBase, public DataBaseWithChildren {
 public:
  ITestNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getData() = 0;
  virtual std::string getValue() = 0;
  virtual IArrayNode* getChildren() = 0;
  virtual ~ITestNode();
};

class IInjectionPointNode : public DataBase, public DataBaseWithChildren {
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

class IUnitTestNode : public DataBase, public DataBaseWithChildren {
 public:
  IUnitTestNode();
  virtual std::string getPackage() = 0;
  virtual IArrayNode* getChildren() = 0;
  virtual IMapNode* getResults() = 0;
  virtual std::string getValue() = 0;
  virtual std::string getTestTemplate(std::string name) = 0;
  virtual ~IUnitTestNode();
};


class IRootNode : public DataBase, public DataBaseWithChildren {
 public:
  IRootNode();
  virtual IArrayNode* getChildren() = 0;
  virtual IArrayNode* getUnitTests() = 0;
  virtual IInfoNode* getInfoNode() = 0;
  virtual ICommInfoNode* getCommInfoNode() = 0;

  virtual std::string getIntro() = 0;
  virtual std::string getConclusion() = 0;
  virtual int getWorldSize() = 0;
  virtual std::string getCommMap() = 0;
  virtual DataBase* findById(long id) = 0;

  virtual bool hasId(long id) { return findById(id) != NULL; }

  virtual ~IRootNode();
};

}  // namespace Nodes

}  // namespace VnV
#endif  // NODES_H
