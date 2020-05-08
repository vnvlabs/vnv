#ifndef NODES_H
#define NODES_H

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <memory>

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
class IMapNode;
class IInfoNode;
class ITestNode;
class IInjectionPointNode;
class IUnitTestNode;

class DataBase {
public:

    enum class DataType { Bool, Integer, Float, Double, String, Long, Array, Map, Log, InjectionPoint, Info, Test, UnitTest };

    std::string id;
    std::string name; // name can be assigned, default is to use id (which is unique).
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
    IArrayNode* getAsArrayNode();
    ILogNode* getAsLogNode();
    IInjectionPointNode* getAsInjectionPointNode();
    IInfoNode* getAsInfoNode();
    ITestNode* getAsTestNode();
    IUnitTestNode* getAsUnitTestNode();
    IMapNode* getAsMapNode();

    std::string getId();
    std::string getName();

    virtual ~DataBase();
    virtual std::string toString();
    std::string getTypeStr();
};

class IMapNode : public DataBase {
public:
     IMapNode();
     virtual DataBase* get(std::string key) = 0;
     virtual IMapNode* add(std::string key, std::shared_ptr<DataBase> v) = 0;
     virtual bool contains(std::string key) = 0;
     virtual std::vector<std::string> fetchkeys() = 0;
     virtual std::size_t size() = 0;
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

     void iter(std::function<void(DataBase*)> &lambda);

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
     virtual double getValue()=0;
     virtual ~IDoubleNode();
};

class IIntegerNode : public DataBase {
 public:
     IIntegerNode();
     virtual int getValue()=0;
     virtual ~IIntegerNode();
} ;

class ILongNode : public DataBase {
public:
     ILongNode();
     virtual long getValue()=0;
     virtual ~ILongNode();
} ;

class IStringNode : public DataBase {
public:
     IStringNode();
     virtual std::string getValue()=0;
     virtual bool isJson() = 0;
     virtual ~IStringNode();
} ;

class IFloatNode : public DataBase {
public:
     IFloatNode();
     virtual float getValue()=0;
     virtual ~IFloatNode();
} ;

class IInfoNode : public DataBase {
public:
     IInfoNode();
     virtual std::string getTitle()=0;
     virtual long getDate()=0;
     virtual ~IInfoNode();
};

class ITestNode : public DataBase {
public:
     ITestNode();
     virtual std::string getPackage()=0;
     virtual IArrayNode* getData()=0;
     virtual IArrayNode* getChildren()=0;
     virtual ~ITestNode();
};

class IInjectionPointNode : public DataBase {
public:
     IInjectionPointNode();
     virtual std::string getPackage() = 0;
     virtual IArrayNode* getTests() = 0;
     virtual IArrayNode* getChildren() = 0;
     virtual ~IInjectionPointNode();
};

class ILogNode : public DataBase {
public:
    ILogNode();
    virtual std::string getPackage()=0;
    virtual std::string getLevel()=0;
    virtual std::string getMessage()=0;
    virtual std::string getStage()=0;
    virtual ~ILogNode();
};


class IUnitTestNode  : public DataBase {
public:
    IUnitTestNode();
    virtual std::string getPackage()=0;
    virtual IArrayNode* getChildren()= 0;
    virtual IMapNode* getResults() = 0;
    virtual ~IUnitTestNode();
};

class IRootNode {
public:
     IRootNode();
     virtual IArrayNode* getChildren() = 0;
     virtual IArrayNode* getUnitTests() = 0;
     virtual IInfoNode* getInfoNode() = 0;
     virtual std::string toString() = 0;
     virtual ~IRootNode();
};

}

}
#endif // NODES_H
