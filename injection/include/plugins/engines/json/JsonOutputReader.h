#ifndef OUTPUTREADERIMPLB_H
#define OUTPUTREADERIMPLB_H

#include <sstream>
#include "interfaces/nodes/Nodes.h"

namespace VnV {
namespace PACKAGENAME {
namespace Engines {
namespace JsonReader {

using namespace VnV::Nodes;

class MapNode : public IMapNode {
public:

    std::map<std::string, std::shared_ptr<DataBase>> value;

    MapNode();
    virtual DataBase* get(std::string key) override ;
    virtual IMapNode* add(std::string key, std::shared_ptr<DataBase> v) override ;
    virtual bool contains(std::string key);
    virtual std::vector<std::string> fetchkeys() override ;
    virtual std::size_t size();
    virtual std::string toString();
};

class ArrayNode : public IArrayNode {
public:
   ArrayNode();
   std::vector<std::shared_ptr<DataBase>> value;
   virtual DataBase* get(std::size_t idx) override;
   virtual std::size_t size() override ;
   virtual IArrayNode* add(std::shared_ptr<DataBase> data) override ;
   virtual std::string toString();
};

class BoolNode : public IBoolNode {
public:
    bool value;
    BoolNode();
    virtual bool getValue() override;
    virtual std::string toString();
};

class DoubleNode : public IDoubleNode {
public:
    double value;
   DoubleNode( );
   virtual double getValue() override;
   virtual std::string toString();

};

class IntegerNode : public IIntegerNode {
 public:
    IntegerNode( );
   int value;
   virtual int getValue() override;
   virtual std::string toString();

} ;

class LongNode : public ILongNode {
public:
    LongNode();
   long value;
   virtual long getValue() override ;
   virtual std::string toString();

} ;

class StringNode : public IStringNode {
public:
    StringNode( );
   std::string value;
   bool jsonString = false;
   virtual std::string getValue() override;
   virtual bool isJson() override;
   virtual std::string toString();

} ;

class FloatNode : public IFloatNode {
public:
    FloatNode();
   float value;
   virtual float getValue() override;
   virtual std::string toString();

} ;

class InfoNode : public IInfoNode {
public:
    InfoNode();
    std::string title;
    long date;
    virtual std::string getTitle() override;
    virtual long getDate() override;
    virtual std::string toString();

};

class TestNode : public ITestNode {
public:
    std::string name, package;
    std::shared_ptr<ArrayNode> data;
    std::shared_ptr<ArrayNode> children;
    TestNode();
    virtual std::string getPackage() override;
    virtual IArrayNode* getData() override;
    virtual IArrayNode* getChildren() override;

    virtual std::string toString();

};

class InjectionPointNode : public IInjectionPointNode {
public:

    std::shared_ptr<ArrayNode> children;
    std::shared_ptr<ArrayNode> tests;
    std::string name;
    std::string package;

    InjectionPointNode();
    virtual std::string getPackage() override;
    virtual IArrayNode* getTests() override;
    virtual IArrayNode* getChildren() override;

    virtual std::string toString();
};


class LogNode : public ILogNode {
public:
    std::string package,level,stage,message;
    LogNode();
    virtual std::string getPackage() override;
    virtual std::string getLevel() override;
    virtual std::string getMessage() override;
    virtual std::string getStage() override;

    virtual std::string toString();
};


class UnitTestNode  : public IUnitTestNode {
public:
    std::string name,package;
    std::shared_ptr<ArrayNode> children;
    std::shared_ptr<MapNode> resultsMap;
    UnitTestNode( );
    virtual std::string getPackage() override;
    virtual IArrayNode* getChildren() override;
    virtual IMapNode* getResults() override;

    virtual std::string toString();
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
IRootNode* parse(std::string filename);

}
}
}
}

#endif // OUTPUTREADERIMPL_H
