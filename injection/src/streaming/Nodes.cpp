

#include <iostream>
#include <sstream>

#include "streaming/Nodes.h"
#include "base/exceptions.h"
#include "base/stores/WalkerStore.h"
#include "json-schema.hpp"

namespace VnV {
namespace Nodes {

bool DataBase::check(DataType type) { return type == getType(); }

std::string DataBase::getName() { 
  std::string n = getNameInternal();
  return (n.empty()) ? std::to_string(getId()) : n;
}

DataBase::DataBase(DataType type) : dataType(type) {}

DataBase::DataType DataBase::getType() { return dataType;  }

DataBase::DataType DataBase::getDataTypeFromString(std::string s) {
  if (s.compare("") == 0 ){}
  #define X(x,y) else if (s.compare(#x) == 0 ) { return DataType::x; }
  DTYPES
  #undef X
  #define X(x) else if (s.compare(#x) == 0 ) { return DataType::x; }
  STYPES
  RTYPES
  #undef X
  throw INJECTION_EXCEPTION("Unknown Datatype %s", s.c_str());
}


std::string DataBase::getTypeStr() {
  switch (getType()) {
#define X(x, y)     \
  case DataType::x: \
    return #x;
    DTYPES
#undef X
#define X(x)        \
  case DataType::x: \
    return #x;
    STYPES
    RTYPES
#undef X
  default:
    return "Custom";
  }
  throw INJECTION_BUG_REPORT_("Impossible");
}


#define X(x, y)                                                     \
  I##x##Node::I##x##Node() : DataBase(DataBase::DataType::x) {}     \
  I##x##Node::~I##x##Node() {}                                      \
  std::shared_ptr<I##x##Node> DataBase::getAs##x##Node(std::shared_ptr<DataBase> ptr) {                          \
    if (check(DataType::x)) return std::dynamic_pointer_cast<I##x##Node>(ptr); \
    throw INJECTION_EXCEPTION("Invalid Cast to DataType::%s from %s ", #x, dataType);     \
  }
DTYPES
#undef X


std::string IStringNode::valueToString(std::string ind) {
  return ind;
}

std::string IJsonNode::valueToString(std::string ind) {
  return ind;
}
std::string IShapeNode::valueToString(std::shared_ptr<DataBase> ind) {
  return ind->toString();
}


template<typename T, typename V> 
json toJ(std::vector<std::size_t> curr, V* cls, const std::function<json(T)>& func) {
 
  auto &sh = cls->getShape();
  if (sh.size() == 0 ) return func(cls->getScalarValue());
  
  json jj = json::array();
      
  curr.push_back(0);      
  for (int i = 0; i < sh[curr.size()-1]; i++  ) {
      curr.back() = i;
      if ( curr.size() == sh.size() ) {
        jj.push_back(func(cls->getValueByShape(curr)));
      } else {
        jj.push_back(toJ(curr,cls, func));
      }
  }
  return jj;
}


json IStringNode::toJson() {
   return toJ<std::string,IStringNode>({},this, [](std::string x){ return x;}); 
}
json IJsonNode::toJson() {
  return toJ<std::string,IJsonNode>({},this, [](std::string x){ return json::parse(x);}); 
}

json IShapeNode::toJson() {
  return toJ<std::shared_ptr<DataBase>,IShapeNode>({},this, [](std::shared_ptr<DataBase> x){ return x->toJson();}); 
}



#define SDTYPES X(Bool, bool) X(Integer, int) X(Float, float) X(Double, double) X(Long, long)
#define X(x,y) \
    std::string I##x##Node::valueToString(y ind) { return std::to_string(ind);}\
    json I##x##Node::toJson() { return toJ<y,I##x##Node>({},this, [](y op){return op;}); }
SDTYPES
#undef X 
#undef SDTYPES


#define X(x)                                                        \
  I##x##Node::I##x##Node() : DataBase(DataBase::DataType::x) {}     \
  I##x##Node::~I##x##Node() {}                                      \
  std::shared_ptr<I##x##Node> DataBase::getAs##x##Node(std::shared_ptr<DataBase> ptr) {                          \
    if (check(DataType::x)) return std::dynamic_pointer_cast<I##x##Node>(ptr); \
    throw INJECTION_EXCEPTION("Invalid Cast to DataType::%s -> %s", #x, dataType);     \
  }
STYPES
RTYPES
#undef X

DataBase::~DataBase() {}

void IArrayNode::iter(std::function<void(std::shared_ptr<DataBase>)>& lambda) {
  for (std::size_t i = 0; i < size(); i++) {
    lambda(get(i));
  }
}

WalkerWrapper::WalkerWrapper(std::shared_ptr<IWalker> walker, IRootNode* root) : ptr(walker), _rootNode(root) {
  node.reset(new WalkerNode());
}

std::shared_ptr<WalkerNode> WalkerWrapper::next() {
  if (ptr->next(*node)) {
    return node;
  }

  node->item = nullptr;
  node->edges.clear();
  if (!rootNode()->processing() ) {
    node->type = node_type::DONE;
  } else {
    node->type = node_type::WAITING;
    node->edges.clear();
  }
  return node;
}



WalkerWrapper IRootNode::getWalker(std::string package, std::string name, std::string config) {
  nlohmann::json j = nlohmann::json::parse(config);
  
  auto a = WalkerStore::instance().getWalker(package, name, rootNode(), j);
  
  if (a == nullptr) {
    throw INJECTION_EXCEPTION("No Walker with the name %s:%s found", package.c_str(), name.c_str());
  }

  return WalkerWrapper(a, rootNode());
}



 json IWorkflowNode::getDataChildren_(int fileId, int level) {
    
    json j = DataBase::getDataChildren_(fileId, level);
    j.push_back("Package:" + getPackage());
    j.push_back("Info:" + getInfo().dump());
    j.push_back("State:" + getState());
    
    json jj = json::object();
    jj["text"] = "Report";
    jj["children"] = json::array();
    
    for (auto it : listReports()) {
      auto r = getReport(it);
      if (r!=nullptr) {
        int i = getReportFileId(it);
        auto a = r->getAsDataChild(i,level-1);
        a["text"] = it;
        jj["children"].push_back(a);
      }
    }

    j.push_back(jj);
    return j;
    
   }

}  // namespace Nodes
}  // namespace VnV
