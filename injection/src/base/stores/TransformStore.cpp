
/** @file TransformStore.cpp Implementation of the TransformStore class as
 *defined in base/TransformStore.h"
 **/
#include "base/stores/TransformStore.h"

#include <dlfcn.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "base/Utilities.h"
#include "base/parser/JsonSchema.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"
#include "base/exceptions.h"

using nlohmann::json_schema::json_validator;

using namespace VnV;

#include "base/Runtime.h"
BaseStoreInstance(TransformStore)

    Transformer::Transformer(
        std::string from_,
        std::vector<std::pair<std::string, ITransform*>>& trans) {
  from = StringUtils::squash_copy(from_);
  transPath = trans;
}

Transformer::~Transformer() {
  // Free all transforms in reverse order
  for (auto it = transPath.rbegin(); it != transPath.rend(); it++) {
    delete it->second;
  }
}

void* Transformer::Transform(void* ptr) {
  for (auto it = transPath.begin(); it != transPath.end(); it++) {
    try {
      ptr = it->second->Transform(ptr);
    } catch (VnV::VnVExceptionBase &e ) {
      ptr = nullptr;
    }
    if (ptr == nullptr) {
      throw INJECTION_EXCEPTION_("Transform Failed");
    }
  }
  return ptr;
}

TransformStore::TransformStore() {}

nlohmann::json TransformStore::schema() {
  std::set<std::string> nodes;
  nlohmann::json edges = json::array();
  for (auto& it : trans_map) {
    nodes.insert(it.first);
    for (auto& itt : it.second) {
      nodes.insert(itt.first);
      json j = json::object();
      j["source"] = it.first;
      j["target"] = it.second;
      edges.push_back(j);
    }
  }
  nlohmann::json r = json::object();
  r["nodes"] = nodes;
  r["links"] = edges;
  return r;
}

std::shared_ptr<Transformer> TransformStore::getTransformer(std::string from,
                                                            std::string to) {
  // Squash the types to remove any whitespace.
  StringUtils::squash(from);
  StringUtils::squash(to);

  std::vector<std::pair<std::string, ITransform*>> m;
  try {
    std::vector<std::pair<std::string, std::string>> r =
        bfs(trans_map, from, to);

    for (auto it : r) {
      m.push_back({it.first, (*trans_factory.find(it.second)->second)()});
    }
    return std::make_shared<Transformer>(from, m);
  } catch (std::exception &e) {
    return nullptr;
  }
}

void TransformStore::print() {
  for (auto it : trans_factory) {
    VnV_Info(VNVPACKAGENAME, "Transform: %s", it.first.c_str());
  }
}

void TransformStore::addTransform(std::string name, trans_ptr t,
                                  std::string from, std::string to) {
  StringUtils::squash(from);
  StringUtils::squash(to);

  trans_factory.insert(std::make_pair(name, t));
  auto it = trans_map.find(from);
  if (it == trans_map.end()) {
    trans_map.insert(
        std::make_pair(from, std::map<std::string, std::string>()));
  }
  trans_map[from][to] = name;
}

void VnV::registerTransform(std::string name, trans_ptr t, std::string from,
                            std::string to) {
  TransformStore::instance().addTransform(name, t, from, to);
}
