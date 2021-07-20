#include "interfaces/nodes/Nodes.h"

#include <iostream>
#include <sstream>

#include "base/exceptions.h"
#include "json-schema.hpp"

namespace VnV {
namespace Nodes {

long DataBase::getId() { std::cout << "Getting Id " << id << std::endl; return id; }

bool DataBase::check(DataType type) { return type == dataType; }

std::string DataBase::getName() {
  return (name.empty()) ? std::to_string(id) : name;
}

DataBase::DataType DataBase::getType() { return dataType; }

DataBase::DataBase(DataType type) : dataType(type) {}

std::string DataBase::getTypeStr() {
  switch (dataType) {
  #define X(x,y) case DataType::x: return #x;
  DTYPES
  #undef X
  #define X(x) case DataType::x: return #x;
  STYPES
  RTYPES
  #undef X
  default:
    return "Custom";
  }
  throw VnVExceptionBase("Impossible");
}

#define X(x,y) \
I##x##Node::I##x##Node() : DataBase(DataBase::DataType::x) {}\
I##x##Node::~I##x##Node() {}                                 \
I##x##Node* DataBase::getAs##x##Node() { \
  return (check(DataType::x)) ? dynamic_cast<I##x##Node*>(this) : nullptr; \
}
DTYPES
#undef X

#define X(x) \
I##x##Node::I##x##Node() : DataBase(DataBase::DataType::x) {}\
I##x##Node::~I##x##Node() {}                                 \
I##x##Node* DataBase::getAs##x##Node() { \
  return (check(DataType::x)) ? dynamic_cast<I##x##Node*>(this) : nullptr; \
}
STYPES
RTYPES
#undef X


DataBase::~DataBase() {}

void IArrayNode::iter(std::function<void(DataBase*)>& lambda) {
  for (std::size_t i = 0; i < size(); i++) {
    lambda(get(i));
  }
}

}  // namespace Nodes
}  // namespace VnV
