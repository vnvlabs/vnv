#ifndef VVSERIALIZERS_H
#define VVSERIALIZERS_H


#include <map>
#include <string>

#include "VnV-Interfaces.h"

namespace VnV {

class SerializerStore {
 private:
  std::map<std::string, std::pair<serializer_ptr*, declare_serializer_ptr*>, std::less<std::string>> serializer_factory;
  SerializerStore();

 public:
  void addSerializer(std::string name, serializer_ptr m, declare_serializer_ptr v);

  ISerializer* getSerializer(std::string name);

  static SerializerStore& getSerializerStore();

  void print();


};

}  // namespace VnV



#endif // VVSERIALIZERS_H
