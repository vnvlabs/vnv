#include "plugins/readers/BasicOutputReaderImpl.h"

namespace VnV{
namespace Reader {

IRootNode* getRootNode() {
     return Impl::readOutputFile("sfdsf","sdfsdf");
}

namespace Impl {
namespace {

template <typename T>
std::shared_ptr<T> mks(T* base) {
     std::shared_ptr<T> base_ptr(base);
     return base_ptr;
}

std::shared_ptr<IUnitTestNode> genUnitTestNode(std::string name, std::string package) {
     std::shared_ptr<UnitTestNode> n = mks(new UnitTestNode());
     n->name = name;
     n->package = package;

     n->children = mks( (ArrayNode*) (new ArrayNode())->add(mks(new DocumentationNode()))
                                        ->add(mks(new DocumentationNode())));

     n->resultsMap = mks( (MapNode*) (new MapNode())->add("r1",mks(new BoolNode()))
                                        ->add("r2",mks(new BoolNode())));
     return n;
}

std::shared_ptr<TestNode> genTestNode() {
     std::shared_ptr<TestNode> t = mks(new TestNode());
     t->name = "Test";
     t->package = "Pac";

     t->data = mks((ArrayNode*) (new ArrayNode())->add( mks(new DoubleNode()))
                                    ->add(mks(new FloatNode()))
                                    ->add(mks(new StringNode())));

     t->children = mks((ArrayNode*)(new ArrayNode())->add(mks(new LogNode()))
                       ->add(mks(new LogNode()))
                       ->add(mks(new DocumentationNode)));
     return t;
}

std::shared_ptr<InjectionPointNode> genInjectionPointNode(int level) {
     std::shared_ptr<InjectionPointNode> ip = mks(new InjectionPointNode());
     ip->name = "Name";
     ip->package = "Package";
     ip->tests = mks((ArrayNode*) (new ArrayNode)->add(genTestNode())
                                     ->add(genTestNode())
                                     ->add(genTestNode()));

     ip->children = mks((ArrayNode*) (new ArrayNode)->add(mks(new DocumentationNode))
                                       ->add(mks(new LogNode())));
     if (level < 2) {
          ip->children->add(genInjectionPointNode(++level));
     }
     return ip;
}

}

IRootNode* readOutputFile(std::string reader, std::string filename){
     RootNode* root =  new RootNode();
     root->children = mks((ArrayNode*) (new ArrayNode())
               ->add( mks(new DocumentationNode()))
               ->add( mks(new LogNode()))
               ->add( genInjectionPointNode(0))
               ->add( genInjectionPointNode(0))
               ->add( genInjectionPointNode(0))
               ->add( genInjectionPointNode(0)));


     root->unitTests = mks((ArrayNode*) (new ArrayNode())
                           ->add( genUnitTestNode("sfd","sdfS"))
                           ->add( genUnitTestNode("sfd","sdfS"))
                           ->add( genUnitTestNode("sfd","sdfS")) );

     root->infoNode = mks(new InfoNode());
     return root;
}



DoubleNode::DoubleNode() {}

double DoubleNode::getValue() {return value;}

TestNode::TestNode() {}

std::string TestNode::getPackage(){return package;}

std::string TestNode::getName() {return name;}

IArrayNode *TestNode::getData() {return data.get();}

IArrayNode *TestNode::getChildren() {return children.get();}

UnitTestNode::UnitTestNode() {}

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

LogNode::LogNode() {}

std::string LogNode::getPackage() {return package;}

std::string LogNode::getLevel() {return level;}

std::string LogNode::getMessage() {return message;}

std::string LogNode::getStage() {return stage;}

DocumentationNode::DocumentationNode()  {}

std::string DocumentationNode::getPackage(){ return package;}

std::string DocumentationNode::getName() {return name;}

IArrayNode *DocumentationNode::getData() {return data.get();}

InjectionPointNode::InjectionPointNode()  {}

std::string InjectionPointNode::getPackage() {return package;}

std::string InjectionPointNode::getName() {return name;}

IArrayNode *InjectionPointNode::getTests() {return tests.get();}

IArrayNode *InjectionPointNode::getChildren() {return children.get();}

InfoNode::InfoNode() {}

std::string InfoNode::getTitle() {return title;}

long InfoNode::getDate() {return date;}

ArrayNode::ArrayNode() {}

DataBase *ArrayNode::get(std::size_t idx) { return (idx < value.size() ) ? (value[idx]).get() : nullptr; }

FloatNode::FloatNode(){}

float FloatNode::getValue() { return value;}

StringNode::StringNode() {}

std::string StringNode::getValue() {return value;}

LongNode::LongNode() {}

long LongNode::getValue() {return value;}

IntegerNode::IntegerNode() {}

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

BoolNode::BoolNode() {}


bool BoolNode::getValue() { return value;}

MapNode::MapNode()  {}

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

Reader::IRootNode *BasicOutputReaderImpl::readFromFile(std::string config) {
    return Reader::Impl::readOutputFile(config,"sdfsdf");;
}

}
