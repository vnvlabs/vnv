#ifndef IOUTPUTREADER_H
#define IOUTPUTREADER_H

#include <string>
#include <vector>

namespace VnV {
namespace Reader {

class DataNode {
   virtual double getAsDouble() = 0;
   virtual int getAsInteger() = 0;
   virtual float getAsFloat() = 0;
   virtual long getAsLong() = 0;
   virtual std::string getAsString() = 0;
   virtual json getAsJson() = 0;
   virtual std::vector<double>& getAsDoubleVector() = 0;
   virtual std::vector<int>& getAsIntegerVector() = 0;
   virtual std::vector<float>& getAsFloatVector() = 0;
   virtual std::vector<long>& getAsLongVector() = 0;
   virtual std::vector<std::string>& getAsStringVector() = 0;
};

class InfoNode {
    virtual std::string getTitle() = 0;
    virtual long getDate() = 0;

};

class DataArray {
    std::vector<DataNode>& getDataArray() = 0;
};

class DocumentationNode {
    virtual std::string getPackage() = 0;
    virtual std::string getName() = 0;
    virtual DataArray& getData() = 0;
};

class LogNode {
    virtual std::string getPackage() = 0;
    virtual std::string getLevel() = 0;
    virtual std::string getMessage() = 0 ;
    virtual std::string getStage() = 0;
};

class InjectionPointNode {
    virtual std::string getPackage() = 0;
    virtual std::string getName() = 0;
    virtual TestNodeArray& getTests() = 0;
    virtual ChildrenArray& getChildren() = 0;
};

class TestNode {
    virtual std::string getPackage() = 0;
    virtual std::string getName() = 0;
    virtual DataArray& getData() = 0;
    virtual ChildrenArray& getChildren() = 0;
};

class TestNodeArray {
    virtual std::vector<TestNode>& getTests() = 0;
};

class Node {
    virtual DocumentationNode getAsDocuementationNode() = 0;
    virtual LogNode getAsLogNode() = 0;
    virtual InjectionPointNode getAsInjectionPointNode() = 0;
};

class ChildrenArray {
public:
    virtual std::vector<Node>& getNodes() = 0;
};

class UnitTestResultsMap {
   virtual std::map<std::string,bool>& getResults() = 0;
};

class UnitTestNode {
    virtual std::string getName()=0;
    virtual string::string getPackage() = 0;
    virtual ChildrenArray& getChildren() = 0;
    virtual UnitTestResultsMap& getResults() = 0;
};

class UnitTestArray {
    virtual std::vector<UnitTestNode>& getUnitTests() = 0;
};

class RootNode {
    virtual ChildrenArray& getchildren() = 0;
    virtual UnitTestArray& getUnitTests() = 0;
    virtual InfoNode& getInfoNode() = 0;
};

OutputEngineReader* readOutputFile(std::string reader, std::string filename) {
   return nullptr;
}

}
}
#endif // IOUTPUTREADER_H
