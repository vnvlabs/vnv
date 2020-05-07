#include "plugins/engines/json/JsonOutputReader.h"
#include <random>
#include <iostream>

namespace VnV{
namespace Engines {
namespace JsonReader {

DoubleNode::DoubleNode() : IDoubleNode(){}

double DoubleNode::getValue() {return value;}

std::string DoubleNode::toString() {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

TestNode::TestNode(): ITestNode() {}

std::string TestNode::getPackage(){return package;}

IArrayNode *TestNode::getData() {return data.get();}

IArrayNode *TestNode::getChildren() {return children.get();}

std::string TestNode::toString() {
    std::ostringstream oss;
    oss << "Test: " << getPackage() << " : " << getName() << "\n";
    oss << "Available Data:\n\n";
    if (getData() != nullptr)
        oss << getData()->toString();
    oss << "Available Children:\n\n";
    if (getChildren() != nullptr)
        oss << getChildren()->toString();
    return oss.str();
}

UnitTestNode::UnitTestNode() : IUnitTestNode(){}

std::string UnitTestNode::getPackage() {return package;}

IArrayNode *UnitTestNode::getChildren() {return children.get();}

IMapNode *UnitTestNode::getResults() { return resultsMap.get();}

std::string UnitTestNode::toString() {
    std::ostringstream oss;
    oss << "InjectionPoint: " << getPackage() << " : " << getName() << "\n";
    oss << "Available Results:\n";

    if (getResults() != nullptr)
        oss << getResults()->toString();

    oss << "Available Children:\n\n";
    if (getChildren() != nullptr)
        oss << getChildren()->toString();
    return oss.str();
}

IArrayNode *RootNode::getChildren() {return children.get();}

IArrayNode *RootNode::getUnitTests() { return unitTests.get(); }

IInfoNode *RootNode::getInfoNode() {return infoNode.get();}

std::string RootNode::toString() {
    std::ostringstream oss;
    oss << "RootNode:\n";
    if (getInfoNode() != nullptr)
        oss << "Info:\n" << getInfoNode()->toString();

    oss << "Available Unit Tests:\n\n";
    if (getUnitTests() != nullptr)
        oss << getUnitTests()->toString();

    oss << "Available Children:\n\n";
    if (getChildren() != nullptr)
        oss << getChildren()->toString();
    return oss.str();
}

std::size_t ArrayNode::size() {return value.size();}

IArrayNode* ArrayNode::add(std::shared_ptr<DataBase> data) {
    value.push_back(data);
    return this;
}

std::string ArrayNode::toString() {
    std::ostringstream oss;
    oss << "[\n";
    for ( auto &it : value ) {
        oss << it->toString() << ",\n";
    }
    oss << "]";
    return oss.str();
}


LogNode::LogNode(): ILogNode() {}

std::string LogNode::getPackage() {return package;}

std::string LogNode::getLevel() {return level;}

std::string LogNode::getMessage() {return message;}

std::string LogNode::getStage() {return stage;}

std::string LogNode::toString() {
    std::ostringstream oss;
    oss << "LogNode: " << getPackage() << " : " << getStage() << " : " << getLevel() << "\n";
    oss << "Message:\n\n\t" << getMessage() << "\n\n";
    return oss.str();
}

DocumentationNode::DocumentationNode() : IDocumentationNode() {}

std::string DocumentationNode::getPackage(){ return package;}

IArrayNode *DocumentationNode::getData() {return data.get();}

std::string DocumentationNode::toString() {
    std::ostringstream oss;
    oss << "Documentation Point: " << getPackage() << " : " << getName() << "\n";
    oss << "Available Data:\n\n";
    if (getData() != nullptr)
        oss << getData()->toString();

    return oss.str();
}

InjectionPointNode::InjectionPointNode() : IInjectionPointNode() {}

std::string InjectionPointNode::getPackage() {return package;}

IArrayNode *InjectionPointNode::getTests() {return tests.get();}

IArrayNode *InjectionPointNode::getChildren() {return children.get();}

std::string InjectionPointNode::toString() {
    std::ostringstream oss;
    oss << "InjectionPoint: " << getPackage() << " : " << getName() << "\n";
    oss << "Available Tests:\n\n";
    if (getTests() != nullptr)
        oss << getTests()->toString();

    oss << "Available Children:\n\n";
    if (getChildren() != nullptr)
        oss << getChildren()->toString();
    return oss.str();
}

InfoNode::InfoNode(): IInfoNode() {}

std::string InfoNode::getTitle() {return title;}

long InfoNode::getDate() {return date;}

std::string InfoNode::toString() {
    std::ostringstream oss;
    oss << getTitle() << " (" << getDate() << ")";
    return oss.str();
}

ArrayNode::ArrayNode(): IArrayNode() {}

DataBase *ArrayNode::get(std::size_t idx) { return (idx < value.size() ) ? (value[idx]).get() : nullptr; }

FloatNode::FloatNode(): IFloatNode(){}

float FloatNode::getValue() { return value;}

std::string FloatNode::toString() {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

StringNode::StringNode(): IStringNode() {}

bool StringNode::isJson() {
    return jsonString;
}

std::string StringNode::getValue() {return value;}

std::string StringNode::toString() {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

LongNode::LongNode(): ILongNode() {}

long LongNode::getValue() {return value;}

std::string LongNode::toString() {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

IntegerNode::IntegerNode() : IIntegerNode() {}

int IntegerNode::getValue() {return value;}

std::string IntegerNode::toString() {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::vector<std::string> MapNode::fetchkeys() {
    std::vector<std::string> v;
    for(auto it = value.begin(); it != value.end(); ++it) {
          v.push_back(it->first);
     }
     return v;
}

std::size_t MapNode::size() { return value.size();}

std::string MapNode::toString() {
    std::ostringstream oss;
    oss << "{\n";
    for ( auto &it : value ) {
        oss << "\t" << it.first << " : " << it.second->toString() << ",\n";
    }
    oss << "}";
    return oss.str();
}

bool MapNode::contains(std::string key) {
    return value.find(key) != value.end();
}

BoolNode::BoolNode() : IBoolNode() {}


bool BoolNode::getValue() { return value;}

std::string BoolNode::toString() {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

MapNode::MapNode() :IMapNode() {}

DataBase *MapNode::get(std::string key) {
    auto it = value.find(key);
     return (it == value.end() ) ? nullptr : (it->second).get();
}

IMapNode* MapNode::add(std::string key, std::shared_ptr<DataBase> v) {
     value.insert(std::make_pair(key, v));
     return this;
}


}
}
}
