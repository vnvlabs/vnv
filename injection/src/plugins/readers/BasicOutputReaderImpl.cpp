#include "plugins/readers/BasicOutputReaderImpl.h"
#include <random>
#include <iostream>

namespace VnV{
namespace Reader {

IRootNode* getRootNode() {
     return Impl::readOutputFile("sfdsf","sdfsdf");
}

namespace Impl {
namespace {

static long idCounter = 0;

std::string random_string(std::size_t length)
{
    return std::to_string(idCounter++);
}

template <typename T>
std::shared_ptr<T> mks(T* base) {
     std::shared_ptr<T> base_ptr(base);
     return base_ptr;
}

std::shared_ptr<IUnitTestNode> genUnitTestNode(std::string name, std::string package) {
     std::shared_ptr<UnitTestNode> n = mks(new UnitTestNode(random_string(10)));
     n->name = name;
     n->package = package;

     n->children = mks( (ArrayNode*) (new ArrayNode(random_string(10)))->add(mks(new DocumentationNode(random_string(10))))
                                        ->add(mks(new DocumentationNode(random_string(10)))));

     n->resultsMap = mks( (MapNode*) (new MapNode(random_string(10)))->add("r1",mks(new BoolNode(random_string(10))))
                                        ->add("r2",mks(new BoolNode(random_string(10)))));
     return n;
}

std::shared_ptr<TestNode> genTestNode() {
     std::shared_ptr<TestNode> t = mks(new TestNode(random_string(10)));
     t->name = "Test";
     t->package = "Pac";

     t->data = mks((ArrayNode*) (new ArrayNode(random_string(10)))->add( mks(new DoubleNode(random_string(10))))
                                    ->add(mks(new FloatNode(random_string(10))))
                                    ->add(mks(new StringNode(random_string(10)))));

     t->children = mks((ArrayNode*)(new ArrayNode(random_string(10)))->add(mks(new LogNode(random_string(10))))
                       ->add(mks(new LogNode(random_string(10))))
                       ->add(mks(new DocumentationNode(random_string(10)))));
     return t;
}

std::shared_ptr<InjectionPointNode> genInjectionPointNode(int level) {
     std::shared_ptr<InjectionPointNode> ip = mks(new InjectionPointNode(random_string(10)));
     ip->name = "Name";
     ip->package = "Package";
     ip->tests = mks((ArrayNode*) (new ArrayNode(random_string(10)))->add(genTestNode())
                                     ->add(genTestNode())
                                     ->add(genTestNode()));

     ip->children = mks((ArrayNode*) (new ArrayNode(random_string(10)))->add(mks(new DocumentationNode(random_string(10))))
                                       ->add(mks(new LogNode(random_string(10)))));
     if (level < 2) {
          ip->children->add(genInjectionPointNode(++level));
     }
     return ip;
}

}

IRootNode* readOutputFile(std::string reader, std::string filename){
     std::cout << "This reader does not actually read the file. It just creates"
                  "a random one for testing. ToDo, implement this reader for the "
                  "json debug engine.";
     RootNode* root =  new RootNode();
     root->children = mks((ArrayNode*) (new ArrayNode(random_string(10)))
               ->add( mks(new DocumentationNode(random_string(10))))
               ->add( mks(new LogNode(random_string(10))))
               ->add( genInjectionPointNode(0))
               ->add( mks(new LogNode(random_string(10))))
               ->add( genInjectionPointNode(0))
               ->add( mks(new DocumentationNode(random_string(10))))
               ->add( genInjectionPointNode(0))
               ->add( mks(new DocumentationNode(random_string(10))))
               ->add( genInjectionPointNode(0)));

     root->unitTests = mks((ArrayNode*) (new ArrayNode(random_string(10)))
                           ->add( genUnitTestNode("package1","name1"))
                           ->add( genUnitTestNode("package1","name2"))
                           ->add( genUnitTestNode("package2","name1")) );

     root->infoNode = mks(new InfoNode(random_string(10)));
     return root;
}



DoubleNode::DoubleNode(std::string id ) : IDoubleNode(id){}

double DoubleNode::getValue() {return value;}

TestNode::TestNode(std::string id ): ITestNode(id) {}

std::string TestNode::getPackage(){return package;}

std::string TestNode::getName() {return name;}

IArrayNode *TestNode::getData() {return data.get();}

IArrayNode *TestNode::getChildren() {return children.get();}

UnitTestNode::UnitTestNode(std::string id ) : IUnitTestNode(id){}

std::string UnitTestNode::getName() {return name;}

std::string UnitTestNode::getPackage() {return package;}

IArrayNode *UnitTestNode::getChildren() {return children.get();}

IMapNode *UnitTestNode::getResults() { return resultsMap.get();}

IArrayNode *RootNode::getChildren() {return children.get();}

IArrayNode *RootNode::getUnitTests() { return unitTests.get(); }

IInfoNode *RootNode::getInfoNode() {return infoNode.get();}

std::string RootNode::toString() {
     std::ostringstream oss;
     oss << "To String not implemented Yet";
     return oss.str();
}

std::size_t ArrayNode::size() {return value.size();}

IArrayNode* ArrayNode::add(std::shared_ptr<DataBase> data) {
     value.push_back(data);
     return this;
}

LogNode::LogNode(std::string id ): ILogNode(id) {}

std::string LogNode::getPackage() {return package;}

std::string LogNode::getLevel() {return level;}

std::string LogNode::getMessage() {return message;}

std::string LogNode::getStage() {return stage;}

DocumentationNode::DocumentationNode(std::string id ) : IDocumentationNode(id) {}

std::string DocumentationNode::getPackage(){ return package;}

std::string DocumentationNode::getName() {return name;}

IArrayNode *DocumentationNode::getData() {return data.get();}

InjectionPointNode::InjectionPointNode(std::string id ) : IInjectionPointNode(id) {}

std::string InjectionPointNode::getPackage() {return package;}

std::string InjectionPointNode::getName() {return name;}

IArrayNode *InjectionPointNode::getTests() {return tests.get();}

IArrayNode *InjectionPointNode::getChildren() {return children.get();}

InfoNode::InfoNode(std::string id ): IInfoNode(id) {}

std::string InfoNode::getTitle() {return title;}

long InfoNode::getDate() {return date;}

ArrayNode::ArrayNode(std::string id ): IArrayNode(id) {}

DataBase *ArrayNode::get(std::size_t idx) { return (idx < value.size() ) ? (value[idx]).get() : nullptr; }

FloatNode::FloatNode(std::string id ): IFloatNode(id){}

float FloatNode::getValue() { return value;}

StringNode::StringNode(std::string id ): IStringNode(id) {}

std::string StringNode::getValue() {return value;}

LongNode::LongNode(std::string id ): ILongNode(id) {}

long LongNode::getValue() {return value;}

IntegerNode::IntegerNode(std::string id ) : IIntegerNode(id) {}

int IntegerNode::getValue() {return value;}

std::vector<std::string> MapNode::fetchkeys() {
     std::vector<std::string> v;
     for(auto it = value.begin(); it != value.end(); ++it) {
          v.push_back(it->first);
     }
     return v;
}

std::size_t MapNode::size() { return value.size();}

bool MapNode::contains(std::string key) {
     return value.find(key) != value.end();
}

BoolNode::BoolNode(std::string id ) : IBoolNode(id) {}


bool BoolNode::getValue() { return value;}

MapNode::MapNode(std::string id ) :IMapNode(id) {}

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

BasicOutputReaderImpl::BasicOutputReaderImpl(){}

Reader::IRootNode *BasicOutputReaderImpl::readFromFile(std::string filename, std::string config) {
    //TODO This should read the file.
    return Reader::Impl::readOutputFile(config,filename);
}

}
