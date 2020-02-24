
/** @file SerializerStore.cpp Implementation of the SerializerStore class as defined in
 * base/SerializerStore.h"
 **/


#include <iostream>

#include "base/SerializerStore.h"
#include "c-interfaces/Logging.h"
#include "interfaces/ISerializer.h"

using namespace VnV;

SerializerStore::SerializerStore() {}

SerializerStore& SerializerStore::getSerializerStore() {
  static SerializerStore store;
  return store;
}

void SerializerStore::addSerializer(std::string name, serializer_ptr m, declare_serializer_ptr v) {
  serializer_factory.insert(std::make_pair(name, std::make_pair(m,v)));
}

ISerializer* SerializerStore::getSerializer(std::string name) {
  auto it = serializer_factory.find(name);
  if (it != serializer_factory.end()) {
    return it->second.first();
  }
  return nullptr;
}


void SerializerStore::print() {
    int b = VnV_BeginStage("Registered Serializers");
    for ( auto it : serializer_factory ) {
        VnV_Info("Serializer: %s ", it.first.c_str());
        int a = VnV_BeginStage("Supports Serialization of: " );
        json j = it.second.second();
        for ( auto s : j["serializes"].items()) {
            VnV_Info("%s", s.value().get<std::string>().c_str());
        }
        VnV_EndStage(a);
        a = VnV_BeginStage("Supports DeSerialization of: " );
        for ( auto s : j["deserializes"].items()) {
            VnV_Info("%s", s.value().get<std::string>().c_str());
        }
        VnV_EndStage(a);
    }
    VnV_EndStage(b);
}

void VnV::registerSerializer(std::string name, serializer_ptr m, declare_serializer_ptr v) {
  SerializerStore::getSerializerStore().addSerializer(name, m,v);
}
