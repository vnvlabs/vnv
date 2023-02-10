#include "interfaces/IDataType.h"

#include <iostream>

#include "base/exceptions.h"
#include "base/stores/DataTypeStore.h"
#include "interfaces/IOutputEngine.h"

void VnV::IDataType::setKey(long long key) { this->key = key; }

long long VnV::IDataType::getKey() { return key; }

VnV::IDataType::~IDataType() {}

void VnV::registerDataType(std::string packageName, std::string name, VnV::dataType_ptr ptr) {
  VnV::DataTypeStore::instance().addDataType(packageName, name, ptr);
}
