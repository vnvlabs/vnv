#include "interfaces/points/Iteration.h"

#include "base/Runtime.h"
#include "base/stores/IteratorStore.h"

void VnV::CppIteration::Register(const char* package, const char* id,
                                 std::string json) {
  VnV::IteratorStore::instance().registerIterator(package, id, json);
}

void VnV::CppIteration::UnwrapParameterPack(int inputs, NTV& mm, NTV& m) {}

VnV_Iterator VnV::CppIteration::BeginIteration(
    VnV_Comm comm, const char* package, const char* id, const char* fname,
    int line, const DataCallback& callback, int once, NTV& inputs,
    NTV& ouputs) {
  return VnV::RunTime::instance().injectionIteration(
      comm, package, id, fname, line, callback, inputs, ouputs, once);
}

int VnV::CppIteration::Iterate(VnV_Iterator* iterator) {
  return VnV::RunTime::instance().injectionIterationRun(iterator);
}
