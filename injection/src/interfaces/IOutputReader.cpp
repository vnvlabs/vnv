#include "interfaces/IOutputReader.h"
#include "base/Runtime.h"
#include "base/OutputReaderStore.h"

namespace VnV {
namespace Reader {

bool DataBase::check(DataType type) { return type == dataType ; }

DataBase::DataType DataBase::getType() {return dataType;}

DataBase::DataBase(DataType type) : dataType(type) {}

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
     return "Not implemented for this type";
}

ILogNode *DataBase::getAsLogNode() { return (check(DataType::Log)) ? dynamic_cast<ILogNode*>(this) : nullptr ;}

IDocumentationNode *DataBase::getAsDocumentationNode() { return (check(DataType::Documentation)) ? dynamic_cast<IDocumentationNode*>(this) : nullptr ;}

IInjectionPointNode *DataBase::getAsInjectionPointNode() { return (check(DataType::InjectionPoint)) ? dynamic_cast<IInjectionPointNode*>(this) : nullptr ;}

IInfoNode *DataBase::getAsInfoNode() { return (check(DataType::Info)) ? dynamic_cast<IInfoNode*>(this) : nullptr ;}

ITestNode *DataBase::getAsTestNode() { return (check(DataType::Test)) ? dynamic_cast<ITestNode*>(this) : nullptr ;}

IUnitTestNode *DataBase::getAsUnitTestNode() { return (check(DataType::UnitTest)) ? dynamic_cast<IUnitTestNode*>(this) : nullptr ;}

ReaderWrapper::ReaderWrapper(std::string readerName) {
    reader = VnV::OutputReaderStore::instance().getReader(readerName);
}

IRootNode *ReaderWrapper::get(std::string config) { return reader->readFromFile(config); }

TreeWrapper::TreeWrapper(std::string name) {
    generator = OutputReaderStore::instance().getTreeGenerator(name);
}

void TreeWrapper::generateTree(std::string config, IRootNode *node) {
    generator->generateTree(node,config);
}

void VnVFinalize() {
    RunTime::instance().Finalize();
}

void VnVInit(std::vector<char*> args, std::string config) {
    int argv = args.size();
    char** argc = args.data();
    RunTime::instance().Init(&argv, &argc, config, nullptr);
}

IMapNode::IMapNode() : DataBase(DataBase::DataType::Map) {}

IMapNode::~IMapNode(){}

IArrayNode::IArrayNode() : DataBase(DataBase::DataType::Array) {}

IArrayNode::~IArrayNode(){}

IBoolNode::IBoolNode() : DataBase(DataBase::DataType::Bool) {}

IBoolNode::~IBoolNode(){}

IDoubleNode::IDoubleNode() : DataBase(DataBase::DataType::Double) {}

IDoubleNode::~IDoubleNode(){}

IIntegerNode::IIntegerNode() : DataBase(DataBase::DataType::Integer) {}

IIntegerNode::~IIntegerNode(){}

ILongNode::ILongNode() : DataBase(DataBase::DataType::Long) {}

ILongNode::~ILongNode(){}

IStringNode::IStringNode() : DataBase(DataBase::DataType::String) {}

IStringNode::~IStringNode(){}

IFloatNode::IFloatNode() : DataBase(DataBase::DataType::Float) {}

IFloatNode::~IFloatNode(){}

IInfoNode::IInfoNode() : DataBase(DataBase::DataType::Info) {}

IInfoNode::~IInfoNode(){}

ITestNode::ITestNode() : DataBase(DataBase::DataType::Test) {}

ITestNode::~ITestNode(){}

IInjectionPointNode::IInjectionPointNode() : DataBase(DataBase::DataType::InjectionPoint) {}

IInjectionPointNode::~IInjectionPointNode(){}

IDocumentationNode::IDocumentationNode() : DataBase(DataBase::DataType::Documentation) {}

IDocumentationNode::~IDocumentationNode(){}

ILogNode::ILogNode() : DataBase(DataBase::DataType::Log) {}

ILogNode::~ILogNode(){}

IUnitTestNode::IUnitTestNode() : DataBase(DataBase::DataType::UnitTest) {}

IUnitTestNode::~IUnitTestNode(){}

IRootNode::IRootNode(){}

IRootNode::~IRootNode(){}

}

// end REader


//void Initalize(std::string configFile, int * argc, char** argv) {
//   RunTime::instance().Init(argc, &argv, configFile, nullptr);
//}

//void Finalize() {
//    RunTime::instance().Finalize();
//}

//std::shared_ptr<Reader::IReader> getReader(std::string reader) {
//    return VnV::OutputReaderStore::instance().getReader(reader);
//}

//std::shared_ptr<Reader::ITreeGenerator> getTreeGenerator(std::string generator) {
//    return VnV::OutputReaderStore::instance().getTreeGenerator(generator);
//}






}
