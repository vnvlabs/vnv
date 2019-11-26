

/** @file vv-serializer.cpp **/

#include "vv-serializer.h"
#include <iostream>
#include "vv-runtime.h"

using namespace VnV;

SerializerStore::SerializerStore() {}

ISerializer::ISerializer(){};
ISerializer::~ISerializer(){};

SerializerStore& SerializerStore::getSerializerStore() {
  static SerializerStore* store = new SerializerStore();
  return *store;
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
    VnV_BeginStage("Registered Serializers");
    for ( auto it : serializer_factory ) {
        VnV_Info("Serializer: %s ", it.first.c_str());
        VnV_BeginStage("Supports Serialization of: " );
        json j = it.second.second();
        for ( auto s : j["serializes"].items()) {
            VnV_Info("%s", s.value().get<std::string>().c_str());
        }
        VnV_EndStage("");
        VnV_BeginStage("Supports DeSerialization of: " );
        for ( auto s : j["deserializes"].items()) {
            VnV_Info("%s", s.value().get<std::string>().c_str());
        }
    }
    VnV_EndStage("");
}

void VnV_registerSerializer(std::string name, serializer_ptr m, declare_serializer_ptr v) {
  SerializerStore::getSerializerStore().addSerializer(name, m,v);
}
