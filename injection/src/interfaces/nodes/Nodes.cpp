#include "interfaces/nodes/Nodes.h"
#include "base/exceptions.h"

#include <sstream>

namespace VnV {
namespace Nodes {

std::string DataBase::getId() {
    return id;
}

bool DataBase::check(DataType type) { return type == dataType ; }

std::string DataBase::getName() {
    return (name.empty()) ? id : name ;
}

DataBase::DataType DataBase::getType() {return dataType;}

DataBase::DataBase(DataType type) : dataType(type) {}

std::string DataBase::getTypeStr(){
    switch(dataType) {
       case DataType::Bool : return "Bool";
       case DataType::Integer : return "Integer";
       case DataType::Float : return "Float";
       case DataType::Double : return "Double";
       case DataType::String : return "String";
       case DataType::Long : return "Long";
       case DataType::Array : return "Array";
       case DataType::Map : return "Map";
       case DataType::Log : return "Log";
       case DataType::Documentation : return "Documentation";
       case DataType::Info : return "Info";
       case DataType::Test : return "Test";
       case DataType::UnitTest : return "UnitTest";
       default: return "Custom";
    }
    throw VnVExceptionBase("Impossible");
}


IBoolNode *DataBase::getAsBoolNode() { return (check(DataType::Bool)) ? dynamic_cast<IBoolNode*>(this) : nullptr;}

IIntegerNode *DataBase::getAsIntegerNode(){ return (check(DataType::Integer)) ? dynamic_cast<IIntegerNode*>(this) : nullptr;}

IDoubleNode *DataBase::getAsDoubleNode() { return (check(DataType::Double)) ? dynamic_cast<IDoubleNode*>(this): nullptr;}

IStringNode *DataBase::getAsStringNode(){ return (check(DataType::String)) ? dynamic_cast<IStringNode*>(this): nullptr;}

ILongNode *DataBase::getAsLongNode(){ return (check(DataType::Long)) ? dynamic_cast<ILongNode*>(this) : nullptr;}

IFloatNode *DataBase::getAsFloatNode(){ return (check(DataType::Float)) ? dynamic_cast<IFloatNode*>(this) : nullptr;}

IArrayNode *DataBase::getAsArrayNode(){ return (check(DataType::Array)) ? dynamic_cast<IArrayNode*>(this) : nullptr;}

IMapNode *DataBase::getAsMapNode() { return (check(DataType::Map))  ? dynamic_cast<IMapNode*>(this) : nullptr;}

DataBase::~DataBase() {}

std::string DataBase::toString() {
     std::ostringstream oss;
     oss << "DataBase Object{ \"Id=" << getId() << "\" , \"type\"=\"" << getTypeStr() <<"\"}\n";
     return oss.str();
}

ILogNode *DataBase::getAsLogNode() { return (check(DataType::Log)) ? dynamic_cast<ILogNode*>(this) : nullptr ;}

IDocumentationNode *DataBase::getAsDocumentationNode() { return (check(DataType::Documentation)) ? dynamic_cast<IDocumentationNode*>(this) : nullptr ;}

IInjectionPointNode *DataBase::getAsInjectionPointNode() { return (check(DataType::InjectionPoint)) ? dynamic_cast<IInjectionPointNode*>(this) : nullptr ;}

IInfoNode *DataBase::getAsInfoNode() { return (check(DataType::Info)) ? dynamic_cast<IInfoNode*>(this) : nullptr ;}

ITestNode *DataBase::getAsTestNode() { return (check(DataType::Test)) ? dynamic_cast<ITestNode*>(this) : nullptr ;}

IUnitTestNode *DataBase::getAsUnitTestNode() { return (check(DataType::UnitTest)) ? dynamic_cast<IUnitTestNode*>(this) : nullptr ;}

IMapNode::IMapNode( ) : DataBase( DataBase::DataType::Map) {}

IMapNode::~IMapNode(){}

IArrayNode::IArrayNode( ) : DataBase(DataBase::DataType::Array) {}

IArrayNode::~IArrayNode(){}

void IArrayNode::iter(std::function<void (DataBase *)> &lambda) {
    for (std::size_t i = 0 ; i < size(); i++ ) {
        lambda(get(i));
    }
}

IBoolNode::IBoolNode( ) : DataBase(DataBase::DataType::Bool) {}

IBoolNode::~IBoolNode(){}

IDoubleNode::IDoubleNode( ) : DataBase(DataBase::DataType::Double) {}

IDoubleNode::~IDoubleNode(){}

IIntegerNode::IIntegerNode( ) : DataBase(DataBase::DataType::Integer) {}

IIntegerNode::~IIntegerNode(){}

ILongNode::ILongNode( ) : DataBase(DataBase::DataType::Long) {}

ILongNode::~ILongNode(){}

IStringNode::IStringNode( ) : DataBase(DataBase::DataType::String) {}

IStringNode::~IStringNode(){}

IFloatNode::IFloatNode( ) : DataBase(DataBase::DataType::Float) {}

IFloatNode::~IFloatNode(){}

IInfoNode::IInfoNode( ) : DataBase(DataBase::DataType::Info) {}

IInfoNode::~IInfoNode(){}

ITestNode::ITestNode( ) : DataBase(DataBase::DataType::Test) {}

ITestNode::~ITestNode(){}

IInjectionPointNode::IInjectionPointNode() : DataBase(DataBase::DataType::InjectionPoint) {}

IInjectionPointNode::~IInjectionPointNode(){}

IDocumentationNode::IDocumentationNode( ) : DataBase(DataBase::DataType::Documentation) {}

IDocumentationNode::~IDocumentationNode(){}

ILogNode::ILogNode( ) : DataBase(DataBase::DataType::Log) {}

ILogNode::~ILogNode(){}

IUnitTestNode::IUnitTestNode( ) : DataBase(DataBase::DataType::UnitTest) {}

IUnitTestNode::~IUnitTestNode(){}

IRootNode::IRootNode(){}

IRootNode::~IRootNode(){}

}

}
