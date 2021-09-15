#include "interfaces/IReduction.h"

#include <iostream>

#include "base/exceptions.h"
#include "base/stores/ReductionStore.h"
#include "interfaces/IOutputEngine.h"

long long VnV::IReduction::getKey() { return key; }

void VnV::IReduction::setKey(long long key) { this->key = key; }

void VnV::registerReduction(std::string packageName, std::string name,
                            VnV::reduction_ptr ptr) {
  ReductionStore::instance().addReduction(packageName, name, ptr);
}
