#ifndef OUTPUTREADERIMPLB_H
#define OUTPUTREADERIMPLB_H

#include "interfaces/IOutputReader.h"

namespace VnV {
namespace Reader{
namespace Impl {

class MapNode : public IMapNode {
public:

    std::map<std::string, std::shared_ptr<DataBase>> value;

    MapNode();
    virtual DataBase* get(std::string key) override ;
    virtual IMapNode* add(std::string key, std::shared_ptr<DataBase> v) override ;
    virtual bool contains(std::string key);
    virtual std::vector<std::string> fetchkeys() override ;
    virtual std::size_t size();
};

class ArrayNode : public IArrayNode {
public:
   ArrayNode();
   std::vector<std::shared_ptr<DataBase>> value;
   virtual DataBase* get(std::size_t idx) override;
   virtual std::size_t size() override ;
   virtual IArrayNode* add(std::shared_ptr<DataBase> data) override ;
};

class BoolNode : public IBoolNode {
public:
    bool value;
    BoolNode();
    virtual bool getValue() override;
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
} ;

class LongNode : public ILongNode {
public:
    LongNode();
   long value;
   virtual long getValue() override ;
} ;

class StringNode : public IStringNode {
public:
    StringNode();
   std::string value;
   virtual std::string getValue() override;
} ;

class FloatNode : public IFloatNode {
public:
    FloatNode();
   float value;
   virtual float getValue() override;
} ;

class InfoNode : public IInfoNode {
public:
    InfoNode();
    std::string title;
    long date;
    virtual std::string getTitle() override;
    virtual long getDate() override;
};

class TestNode : public ITestNode {
public:
    std::string name, package;
    std::shared_ptr<ArrayNode> data;
    std::shared_ptr<ArrayNode> children;
    TestNode();
    virtual std::string getPackage() override;
    virtual std::string getName() override;
    virtual IArrayNode* getData() override;
    virtual IArrayNode* getChildren() override ;
};

class InjectionPointNode : public IInjectionPointNode {
public:

    std::shared_ptr<ArrayNode> children;
    std::shared_ptr<ArrayNode> tests;
    std::string name;
    std::string package;

    InjectionPointNode();
    virtual std::string getPackage() override;
    virtual std::string getName() override;
    virtual IArrayNode* getTests() override;
    virtual IArrayNode* getChildren() override;
};

class DocumentationNode : public IDocumentationNode {
public:
    std::shared_ptr<ArrayNode> data;
    std::string name;
    std::string package;
    DocumentationNode();
    virtual std::string getPackage() override;
    virtual std::string getName() override;
    virtual IArrayNode* getData() override;
};

class LogNode : public ILogNode {
public:
    std::string package,level,stage,message;
    LogNode();
    virtual std::string getPackage() override;
    virtual std::string getLevel() override;
    virtual std::string getMessage() override;
    virtual std::string getStage() override;
};


class UnitTestNode  : public IUnitTestNode {
public:
    std::string name,package;
    std::shared_ptr<ArrayNode> children;
    std::shared_ptr<MapNode> resultsMap;
    UnitTestNode();
    virtual std::string getName() override;
    virtual std::string getPackage() override;
    virtual IArrayNode* getChildren() override;
    virtual IMapNode* getResults() override;
};

class RootNode : public IRootNode {
public:
    std::shared_ptr<ArrayNode> children;
    std::shared_ptr<ArrayNode> unitTests;
    std::shared_ptr<InfoNode> infoNode;
    virtual IArrayNode* getChildren() override;
    virtual IArrayNode* getUnitTests() override;
    virtual IInfoNode* getInfoNode() override;
    virtual std::string toString() override;
};

// Debug for now
IRootNode* readOutputFile(std::string reader, std::string filename);

}
}

class BasicOutputReaderImpl : public Reader::IReader {
public:
    BasicOutputReaderImpl();
    virtual Reader::IRootNode* readFromFile(std::string config);
};


}


#endif // OUTPUTREADERIMPL_H
