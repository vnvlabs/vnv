#include "interfaces/IDataType.h"

#include <iostream>

#include "shared/exceptions.h"
#include "base/stores/DataTypeStore.h"
#include "interfaces/IOutputEngine.h"



VnV::IDataType::~IDataType() {}

void VnV::registerDataType(std::string packageName, std::string name, VnV::dataType_ptr ptr) {
  VnV::DataTypeStore::instance().addDataType(packageName, name, ptr);
}

VnV::IDataType::IDataType() : key(0) {}


void VnV::IDataType::setData(void* /*data*/) {
    throw INJECTION_EXCEPTION("setData", key);
}

void* VnV::IDataType::getPutData(std::string /*name*/) {
    throw INJECTION_EXCEPTION("getPutData", key);
}

long long VnV::IDataType::maxSize() {
    throw INJECTION_EXCEPTION("maxSize", key);
}

long long VnV::IDataType::pack(void* /*buffer*/) {
    throw INJECTION_EXCEPTION("pack", key);
}

void VnV::IDataType::unpack(void* /*buffer*/) {
    throw INJECTION_EXCEPTION("unpack", key);
}

void VnV::IDataType::axpy(double /*alpha*/, VnV::IDataType_ptr /*y*/) {
    throw INJECTION_EXCEPTION("axpy", key);
}

int VnV::IDataType::compare(IDataType_ptr /*y*/) {
    throw INJECTION_EXCEPTION("compare", key);
    return 0; // Dummy return, should never reach here
}

void VnV::IDataType::mult(IDataType_ptr /*y*/) {
    throw INJECTION_EXCEPTION("mult", key);
}
std::string VnV::IDataType::typeId() {
    throw INJECTION_EXCEPTION("typeid", key);
}

std::string VnV::IDataType::displayName() {
    throw INJECTION_EXCEPTION("displayName", key);
}

void VnV::IDataType::setKey(long long key) { this->key = key; }

long long VnV::IDataType::getKey() { return key; }