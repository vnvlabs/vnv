#ifndef IOUTPUTREADER_H
#define IOUTPUTREADER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include "base/exceptions.h"
//#include "json-schema.hpp"

namespace VnV {

namespace Reader {

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
class IDocumentationNode;
class IInjectionPointNode;
class IUnitTestNode;

class DataBase {
private:
    std::string id;

public:
    enum class DataType { Bool, Integer, Float, Double, String, Long, Array, Map, Log, Documentation, InjectionPoint, Info, Test, UnitTest };
    DataType dataType;	
    bool check(DataType type);
    DataBase(std::string id, DataType type);
    DataType getType();
    IBoolNode* getAsBoolNode();
    IDoubleNode* getAsDoubleNode();
    IIntegerNode* getAsIntegerNode();
    IStringNode* getAsStringNode();
    ILongNode* getAsLongNode();
    IFloatNode* getAsFloatNode();
    IArrayNode* getAsArrayNode();
    ILogNode* getAsLogNode();
    IDocumentationNode* getAsDocumentationNode();
    IInjectionPointNode* getAsInjectionPointNode();
    IInfoNode* getAsInfoNode();
    ITestNode* getAsTestNode();
    IUnitTestNode* getAsUnitTestNode();
    IMapNode* getAsMapNode();

    std::string getId();
    virtual ~DataBase();
    virtual std::string toString();

};

class IMapNode : public DataBase {
public:
     IMapNode(std::string id);
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
     IArrayNode(std::string id);
     virtual DataBase* get(std::size_t idx) = 0;
     virtual std::size_t size() = 0;
     virtual IArrayNode* add(std::shared_ptr<DataBase> data) = 0;
     virtual ~IArrayNode();
};

class IBoolNode : public DataBase {
public:
     IBoolNode(std::string id);
    virtual bool getValue() = 0;
     virtual ~IBoolNode();
};

class IDoubleNode : public DataBase {
public:
     IDoubleNode(std::string id);
     virtual double getValue()=0;
     virtual ~IDoubleNode();
};

class IIntegerNode : public DataBase {
 public:
     IIntegerNode(std::string id);
     virtual int getValue()=0;
     virtual ~IIntegerNode();
} ;

class ILongNode : public DataBase {
public:
     ILongNode(std::string id);
     virtual long getValue()=0;
     virtual ~ILongNode();
} ;

class IStringNode : public DataBase {
public:
     IStringNode(std::string id);
     virtual std::string getValue()=0;
     virtual ~IStringNode();
} ;

class IFloatNode : public DataBase {
public:
     IFloatNode(std::string id);
     virtual float getValue()=0;
     virtual ~IFloatNode();
} ;

class IInfoNode : public DataBase {
public:
     IInfoNode(std::string id);
     virtual std::string getTitle()=0;
     virtual long getDate()=0;
     virtual ~IInfoNode();
};

class ITestNode : public DataBase {
public:
     ITestNode(std::string id);
     virtual std::string getPackage()=0;
     virtual std::string getName()=0;
     virtual IArrayNode* getData()=0;
     virtual IArrayNode* getChildren()=0;
     virtual ~ITestNode();
};

class IInjectionPointNode : public DataBase {
public:
     IInjectionPointNode(std::string id);
     virtual std::string getPackage() = 0;
     virtual std::string getName() = 0;
     virtual IArrayNode* getTests() = 0;
     virtual IArrayNode* getChildren() = 0;
     virtual ~IInjectionPointNode();
};

class IDocumentationNode : public DataBase {
public:
    IDocumentationNode(std::string id);
    virtual std::string getPackage()=0;
    virtual std::string getName()=0;
    virtual IArrayNode* getData()=0;
    virtual ~IDocumentationNode();
};

class ILogNode : public DataBase {
public:
    ILogNode(std::string id);
    virtual std::string getPackage()=0;
    virtual std::string getLevel()=0;
    virtual std::string getMessage()=0;
    virtual std::string getStage()=0;
    virtual ~ILogNode();
};


class IUnitTestNode  : public DataBase {
public:
    IUnitTestNode(std::string id);
    virtual std::string getName() =0;
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

class ITreeGenerator {
public:
    //virtual void generateTree(std::shared_ptr<IRootNode> root, nlohmann::json &config) = 0;
    virtual void generateTree(std::string outputDirectory, const IRootNode* root, std::string config) = 0;
};

class IReader {
public:
    virtual IRootNode* readFromFile(std::string filename, std::string config) = 0;
    //virtual std::shared_ptr<IRootNode> readFromFile(nlohmann::json config) = 0;
};

class ReaderWrapper {
private:
    std::string filename;
    std::string readerConfig;
    std::unique_ptr<IReader> reader;
public:
    ReaderWrapper(std::string readerName, std::string filename, std::string config);
    IRootNode* get();
};

class TreeWrapper {
private:
    std::string treeConfig;
    std::unique_ptr<ITreeGenerator> generator;
public:
    TreeWrapper(std::string name,std::string config);
    void generateTree(std::string outputDirectory, IRootNode* node);
};

void VnVInit(std::vector<std::string> args, std::string config);
void VnVFinalize();

}

}




#endif // IOUTPUTREADER_H
