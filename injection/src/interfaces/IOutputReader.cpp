#include "interfaces/IOutputReader.h"
#include "base/Runtime.h"
#include "base/OutputReaderStore.h"

namespace VnV {
namespace Reader {

std::string DataBase::getId() {
    return id;
}

bool DataBase::check(DataType type) { return type == dataType ; }

DataBase::DataType DataBase::getType() {return dataType;}

DataBase::DataBase(std::string id_, DataType type) : dataType(type), id(id_) {}

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

ReaderWrapper::ReaderWrapper(std::string readerName, std::string filename_, std::string config) : filename(filename_), readerConfig(config) {
    reader = VnV::OutputReaderStore::instance().getReader(readerName);
}

IRootNode *ReaderWrapper::get() { return reader->readFromFile(filename, readerConfig); }

TreeWrapper::TreeWrapper(std::string name,std::string config) {
    generator = OutputReaderStore::instance().getTreeGenerator(name);
    treeConfig = config;
}

void TreeWrapper::generateTree(std::string outputDirectory, IRootNode *node) {
    generator->generateTree(outputDirectory, node,this->treeConfig);
}

void VnVFinalize() {
    RunTime::instance().Finalize();
}

void VnVInit(std::vector<std::string> args, std::string config) {
    int argc = args.size();
    std::vector<char*> cstrings ;
    cstrings.reserve(args.size());
    for (int i = 0; i < args.size(); i++) {
        cstrings.push_back(const_cast<char*>(args[i].c_str()));
    }
    char** argv = &cstrings[0];
    RunTime::instance().Init(&argc, &argv, config, nullptr);
}

IMapNode::IMapNode(std::string id ) : DataBase(id, DataBase::DataType::Map) {}

IMapNode::~IMapNode(){}

IArrayNode::IArrayNode(std::string id ) : DataBase(id,DataBase::DataType::Array) {}

IArrayNode::~IArrayNode(){}

IBoolNode::IBoolNode(std::string id ) : DataBase(id,DataBase::DataType::Bool) {}

IBoolNode::~IBoolNode(){}

IDoubleNode::IDoubleNode(std::string id ) : DataBase(id,DataBase::DataType::Double) {}

IDoubleNode::~IDoubleNode(){}

IIntegerNode::IIntegerNode(std::string id ) : DataBase(id,DataBase::DataType::Integer) {}

IIntegerNode::~IIntegerNode(){}

ILongNode::ILongNode(std::string id ) : DataBase(id,DataBase::DataType::Long) {}

ILongNode::~ILongNode(){}

IStringNode::IStringNode(std::string id ) : DataBase(id,DataBase::DataType::String) {}

IStringNode::~IStringNode(){}

IFloatNode::IFloatNode(std::string id ) : DataBase(id,DataBase::DataType::Float) {}

IFloatNode::~IFloatNode(){}

IInfoNode::IInfoNode(std::string id ) : DataBase(id,DataBase::DataType::Info) {}

IInfoNode::~IInfoNode(){}

ITestNode::ITestNode(std::string id ) : DataBase(id,DataBase::DataType::Test) {}

ITestNode::~ITestNode(){}

IInjectionPointNode::IInjectionPointNode(std::string id ) : DataBase(id,DataBase::DataType::InjectionPoint) {}

IInjectionPointNode::~IInjectionPointNode(){}

IDocumentationNode::IDocumentationNode(std::string id ) : DataBase(id,DataBase::DataType::Documentation) {}

IDocumentationNode::~IDocumentationNode(){}

ILogNode::ILogNode(std::string id ) : DataBase(id,DataBase::DataType::Log) {}

ILogNode::~ILogNode(){}

IUnitTestNode::IUnitTestNode(std::string id ) : DataBase(id,DataBase::DataType::UnitTest) {}

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
