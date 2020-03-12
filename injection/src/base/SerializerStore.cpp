
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
    json k = v();
    for ( auto &it : k["serializes"].items()) {
        serializer_factory.insert(std::make_pair(it.value().get<std::string>(), m));
    }
    for ( auto &it : k["deserializes"].items()) {
        deserializer_factory.insert(std::make_pair(it.value().get<std::string>(), m));
    }
    serializer_name.insert(std::make_pair(name,m));
}

ISerializer* SerializerStore::getSerializerFor(std::string type) {
    auto it = serializer_factory.find(type);
    if (it!=serializer_factory.end() ){
        return it->second();
    }
    return nullptr;
}

ISerializer* SerializerStore::getDeSerializerFor(std::string type) {
    auto it = deserializer_factory.find(type);
    if (it!=deserializer_factory.end() ){
        return it->second();
    }
    return nullptr;
}
ISerializer* SerializerStore::getSerializerByName(std::string name) {
    auto it = serializer_name.find(name);
    if (it!=serializer_name.end() ){
        return it->second();
    }
    return nullptr;
}





void SerializerStore::print() {
    int b = VnV_BeginStage("Registered Serializers");
    for ( auto it : serializer_factory ) {
        VnV_Info("Serializes: %s ", it.first.c_str());
    }
    for ( auto it :  deserializer_factory) {
       VnV_Info("Serializes: %s ", it.first.c_str());
    }
    VnV_EndStage(b);
}

void VnV::registerSerializer(std::string name, serializer_ptr m, declare_serializer_ptr v) {
  SerializerStore::getSerializerStore().addSerializer(name, m,v);
}
