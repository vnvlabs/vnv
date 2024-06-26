﻿#include "base/stores/DataTypeStore.h"

#include <iostream>

#include "base/Runtime.h"
#include "shared/Utilities.h"
#include "shared/exceptions.h"

namespace VnV {

BaseStoreInstance(DataTypeStore)

    namespace {
  long long getKey(std::string packageName, std::string name) {
    return VnV::StringUtils::simpleHash(packageName + ":" + name);
  }

  long long getKey(IDataType_ptr ptr) { return ptr->getKey(); }
}

void DataTypeStore::addDataType(std::string packageName, std::string name, dataType_ptr m) {
  dataType_factory.insert(std::make_pair(getKey("", name), m));
}

IDataType_ptr DataTypeStore::getDataType(long long key) {
  auto it = dataType_factory.find(key);
  if (it != dataType_factory.end()) {
    IDataType_ptr ptr(it->second());
    ptr->setKey(key);
    return ptr;
  }
  throw INJECTION_EXCEPTION("No data type exists %d", key);
}

IDataType_ptr DataTypeStore::getDataType(std::string name) { return getDataType(getKey("", name)); }

}  // namespace VnV
