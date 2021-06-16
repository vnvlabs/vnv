
/**
  @file ITest.cpp
**/
#include "interfaces/IIterator.h"
#include "base/stores/IteratorsStore.h"

using namespace VnV;

void VnV::registerIterator(std::string package, std::string name, std::string schema, iterator_maker_ptr m,
                       std::map<std::string, std::string> map) {
  IteratorsStore::instance().addTest(package, name, schema, m, map);
}