#include "interfaces/nodes/Nodes.h"

#include <iostream>
#include <sstream>

#include "base/exceptions.h"
#include "json-schema.hpp"

namespace VnV {
namespace Nodes {

long DataBase::getId() { return id; }

bool DataBase::check(DataType type) { return type == dataType; }

std::string DataBase::getName() {
  return (name.empty()) ? std::to_string(id) : name;
}

DataBase::DataType DataBase::getType() { return dataType; }

DataBase::DataBase(DataType type) : dataType(type) {}

std::string DataBase::getTypeStr() {
  switch (dataType) {
  case DataType::Bool:
    return "Bool";
  case DataType::Integer:
    return "Integer";
  case DataType::Float:
    return "Float";
  case DataType::Double:
    return "Double";
  case DataType::String:
    return "String";
  case DataType::Long:
    return "Long";
  case DataType::Array:
    return "Array";
  case DataType::Map:
    return "Map";
  case DataType::Log:
    return "Log";
  case DataType::Info:
    return "Info";
  case DataType::ShapeArray:
      return "ShapeArray";
  case DataType::Test:
    return "Test";
  case DataType::UnitTest:
    return "UnitTest";
  case DataType::DataNode:
    return "DataNode";
  default:
    return "Custom";
  }
  throw VnVExceptionBase("Impossible");
}

IBoolNode* DataBase::getAsBoolNode() {
  return (check(DataType::Bool)) ? dynamic_cast<IBoolNode*>(this) : nullptr;
}

IIntegerNode* DataBase::getAsIntegerNode() {
  return (check(DataType::Integer)) ? dynamic_cast<IIntegerNode*>(this)
                                    : nullptr;
}

IDoubleNode* DataBase::getAsDoubleNode() {
  return (check(DataType::Double)) ? dynamic_cast<IDoubleNode*>(this) : nullptr;
}

IStringNode* DataBase::getAsStringNode() {
  return (check(DataType::String)) ? dynamic_cast<IStringNode*>(this) : nullptr;
}

ILongNode* DataBase::getAsLongNode() {
  return (check(DataType::Long)) ? dynamic_cast<ILongNode*>(this) : nullptr;
}

IFloatNode* DataBase::getAsFloatNode() {
  return (check(DataType::Float)) ? dynamic_cast<IFloatNode*>(this) : nullptr;
}

IArrayNode* DataBase::getAsArrayNode() {
  return (check(DataType::Array)) ? dynamic_cast<IArrayNode*>(this) : nullptr;
}

IShapeNode* DataBase::getAsShapeNode() {
  return (check(DataType::ShapeArray)) ? dynamic_cast<IShapeNode*>(this) : nullptr;
}

IMapNode* DataBase::getAsMapNode() {
  return (check(DataType::Map)) ? dynamic_cast<IMapNode*>(this) : nullptr;
}

IDataTypeNode* DataBase::getAsDataTypeNode() {
  return (check(DataType::DataNode)) ? dynamic_cast<IDataTypeNode*>(this)
                                     : nullptr;
}

DataBase::~DataBase() {}

ILogNode* DataBase::getAsLogNode() {
  return (check(DataType::Log)) ? dynamic_cast<ILogNode*>(this) : nullptr;
}

IInjectionPointNode* DataBase::getAsInjectionPointNode() {
  return (check(DataType::InjectionPoint))
             ? dynamic_cast<IInjectionPointNode*>(this)
             : nullptr;
}

IInfoNode* DataBase::getAsInfoNode() {
  return (check(DataType::Info)) ? dynamic_cast<IInfoNode*>(this) : nullptr;
}

ITestNode* DataBase::getAsTestNode() {
  return (check(DataType::Test)) ? dynamic_cast<ITestNode*>(this) : nullptr;
}

IUnitTestNode* DataBase::getAsUnitTestNode() {
  return (check(DataType::UnitTest)) ? dynamic_cast<IUnitTestNode*>(this)
                                     : nullptr;
}

IRootNode* DataBase::getAsRootNode() {
  return (check(DataType::RootNode)) ? dynamic_cast<IRootNode*>(this) : nullptr;
}

IMapNode::IMapNode() : DataBase(DataBase::DataType::Map) {}

IMapNode::~IMapNode() {}

IArrayNode::IArrayNode() : DataBase(DataBase::DataType::Array) {}

IArrayNode::~IArrayNode() {}

void IArrayNode::iter(std::function<void(DataBase*)>& lambda) {
  for (std::size_t i = 0; i < size(); i++) {
    lambda(get(i));
  }
}

IBoolNode::IBoolNode() : DataBase(DataBase::DataType::Bool) {}

IBoolNode::~IBoolNode() {}

IDoubleNode::IDoubleNode() : DataBase(DataBase::DataType::Double) {}

IDoubleNode::~IDoubleNode() {}

IIntegerNode::IIntegerNode() : DataBase(DataBase::DataType::Integer) {}

IIntegerNode::~IIntegerNode() {}

ILongNode::ILongNode() : DataBase(DataBase::DataType::Long) {}

ILongNode::~ILongNode() {}

IStringNode::IStringNode() : DataBase(DataBase::DataType::String) {}

IStringNode::~IStringNode() {}

IFloatNode::IFloatNode() : DataBase(DataBase::DataType::Float) {}

IFloatNode::~IFloatNode() {}

IShapeNode::IShapeNode() : DataBase(DataBase::DataType::ShapeArray) {}
IShapeNode::~IShapeNode() {}

IInfoNode::IInfoNode() : DataBase(DataBase::DataType::Info) {}

IInfoNode::~IInfoNode() {}

ITestNode::ITestNode() : DataBase(DataBase::DataType::Test) {}

ITestNode::~ITestNode() {}

IDataTypeNode::IDataTypeNode() : DataBase(DataBase::DataType::DataNode) {}
IDataTypeNode::~IDataTypeNode() {}

IInjectionPointNode::IInjectionPointNode()
    : DataBase(DataBase::DataType::InjectionPoint) {}

IInjectionPointNode::~IInjectionPointNode() {}

ILogNode::ILogNode() : DataBase(DataBase::DataType::Log) {}

ILogNode::~ILogNode() {}

IUnitTestNode::IUnitTestNode() : DataBase(DataBase::DataType::UnitTest) {}

IUnitTestNode::~IUnitTestNode() {}

IRootNode::IRootNode() : DataBase(DataBase::DataType::RootNode) {}

IRootNode::~IRootNode() {}

DataBase* IShapeNode::get(int index) {

      auto it = getChildren();

      if ( it == nullptr )
        return nullptr;
      return it->get(index);
   }

}  // namespace Nodes

}  // namespace VnV
