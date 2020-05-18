
/**
  @file SerializerStore.h
**/

#ifndef VVSERIALIZERS_H
#define VVSERIALIZERS_H


#include <map>
#include <string>

#include "interfaces/ISerializer.h"

namespace VnV {

class SerializerStore {

 private:
  std::map<std::string, serializer_ptr* , std::less<std::string> > serializer_factory;
  std::map<std::string, serializer_ptr*, std::less<std::string> > serializer_name;
  SerializerStore();

 public:
  void addSerializer(std::string name, serializer_ptr m, std::string type);

  ISerializer* getSerializerFor(std::string name);
  ISerializer* getSerializerByName(std::string name);

  static SerializerStore& getSerializerStore();

  void print();


};

}

#endif // VVSERIALIZERS_H
