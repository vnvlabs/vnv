
#include <dlfcn.h>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <iostream>
#include <sstream>

#include "base/JsonSchema.h"
#include "base/TransformStore.h"
#include "c-interfaces/logging-interface.h"
#include "json-schema.hpp"

using nlohmann::json_schema::json_validator;

using namespace VnV;

TransformStore::TransformStore() {}

TransformStore& TransformStore::getTransformStore() {
  static TransformStore store;
  return store;
}


void* TransformStore::getTransform(std::string from, std::string to, void* ptr) {

   // TODO -- Allow for multiple transforms. Currently, to obtain a transform, we
   // search the trans_map for an entry related to the class "from". If one is
   // found, we search the inner map from an entry named "to". If that is found
   // then the transform with the name trans_map[from][to] has declared that it
   // can do the required transform.

   // To improve this, we could create a directed transform graph. This would allow
   // us to chain together multiple transforms to achieve the desired result.

   // TODO Need to figure out the memeory management for these transforms.

   if  (from.compare(to) == 0) {
       return ptr;
   } else {
     auto t = trans_map.find(from);
     if (t != trans_map.end()) {
       auto tt = t->second.find(to);
       if ( tt != t->second.end()) {
          ITransform *trans = trans_factory.find(tt->second)->second();
          return trans->Transform(to,from,ptr);
       }
     }
     VnV_Warn("Cannot Convert from Object from (%s) to (%s).", from.c_str(),to.c_str());
     return ptr;
   }
}


void TransformStore::print() {
    auto a = VnV_BeginStage("Transform Store Configuration");
    for ( auto it : trans_factory) {
        VnV_Info("Transform: %s", it.first.c_str());
    }

    VnV_EndStage(a);
}

void TransformStore::addTransform(std::string name, trans_ptr t, declare_transform_ptr v) {
  trans_factory.insert(std::make_pair(name,t));
  json x = v();
  json_validator validator;
  validator.set_root_schema(getTransformDeclarationSchema());
  validator.validate(x);

  for( auto from : x.items() ) {
      std::string fromStr = from.key();
      auto from_trans = trans_map.insert(std::make_pair(fromStr,std::map<std::string,std::string>())).first->second;
      for ( auto to : from.value().items() ){
          from_trans.insert(std::make_pair(to.value(),name));
      }
  }
}

void VnV::registerTransform(std::string name, trans_ptr t, declare_transform_ptr v) {
  TransformStore::getTransformStore().addTransform(name, t,v);

}
