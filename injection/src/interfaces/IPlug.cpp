
/**
  @file ITest.cpp
**/
#include "interfaces/IPlug.h"

#include "base/stores/PlugsStore.h"

using namespace VnV;

void VnV::registerPlug(std::string package, std::string name, std::string schema, plug_maker_ptr m) {
  PlugsStore::instance().addTest(package, name, schema, m);
}