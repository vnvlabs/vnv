
/** @file TransformStore.cpp Implementation of the TransformStore class as defined in
 * base/TransformStore.h"
 **/
#include <dlfcn.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <iostream>
#include <sstream>

#include "base/JsonSchema.h"
#include "base/TransformStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"
#include "base/Utilities.h"
using nlohmann::json_schema::json_validator;

using namespace VnV;

Transformer::Transformer(std::string from_, std::vector<std::pair<std::string,ITransform*>> &trans) {
   from = from_;
   transPath = trans;
}

Transformer::~Transformer() {
    //Free all transforms in reverse order
    for ( auto it = transPath.rbegin(); it != transPath.rend(); it++) {
        delete it->second;
    }
}

void* Transformer::Transform(void* ptr, std::string &rtti) {
    std::string f = from;
    for (auto it = transPath.begin(); it != transPath.end(); it++) {
        ptr = it->second->Transform(f,it->first,ptr, rtti);
        f = it->first;
    }
    return ptr;
}

TransformStore::TransformStore() {}

TransformStore& TransformStore::getTransformStore() {
  static TransformStore store;
  return store;
}

std::shared_ptr<Transformer> TransformStore::getTransformer(std::string from, std::string to) {

   std::vector<std::pair<std::string,ITransform*>> m;
   try {
       std::vector<std::pair<std::string,std::string>> r = bfs(trans_map,from,to);
       for (auto it : r) {
           m.push_back({it.first, trans_factory.find(it.second)->second()});
       }
       return std::make_shared<Transformer>(from,m);
   } catch (...) {
        return nullptr;
   }
}


void TransformStore::print() {
    auto a = VnV_BeginStage("Transform Store Configuration");
    for ( auto it : trans_factory) {
        VnV_Info("Transform: %s", it.first.c_str());
    }
    VnV_EndStage(a);
}

void TransformStore::addTransform(std::string name, trans_ptr t, std::string from, std::string to ) {
  trans_factory.insert(std::make_pair(name,t));
  auto it = trans_map.find(from);
  if (it == trans_map.end() ) {
      trans_map.insert(std::make_pair(from,std::map<std::string,std::string>()));
  }
  trans_map[from][to] = name;
}

void VnV::registerTransform(std::string name, trans_ptr t, std::string from, std::string to) {
  TransformStore::getTransformStore().addTransform(name, t,from,to);

}
