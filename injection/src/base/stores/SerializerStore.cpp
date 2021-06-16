
/** @file SerializerStore.cpp Implementation of the SerializerStore class as
 *defined in base/SerializerStore.h"
 **/

#include "base/stores/SerializerStore.h"

#include <iostream>

#include "c-interfaces/Logging.h"
#include "interfaces/ISerializer.h"

using namespace VnV;

SerializerStore::SerializerStore() {}

SerializerStore& SerializerStore::getSerializerStore() {
  static SerializerStore store;
  return store;
}

void SerializerStore::addSerializer(std::string name, serializer_ptr m,
                                    std::string type) {
  serializer_factory.insert(std::make_pair(type, m));
  serializer_name.insert(std::make_pair(name, m));
}

ISerializer* SerializerStore::getSerializerFor(std::string type) {
  auto it = serializer_factory.find(type);
  if (it != serializer_factory.end()) {
    return it->second();
  }
  return nullptr;
}

ISerializer* SerializerStore::getSerializerByName(std::string name) {
  auto it = serializer_name.find(name);
  if (it != serializer_name.end()) {
    return it->second();
  }
  return nullptr;
}

void SerializerStore::print() {
  int b = VnV_BeginStage(VNVPACKAGENAME, "Registered Serializers");
  for (auto it : serializer_factory) {
    VnV_Info(VNVPACKAGENAME, "Serializes: %s ", it.first.c_str());
  }
  VnV_EndStage(VNVPACKAGENAME, b);
}

void VnV::registerSerializer(std::string packageName, std::string name,
                             serializer_ptr m, std::string type) {
  SerializerStore::getSerializerStore().addSerializer(name, m, type);
}
