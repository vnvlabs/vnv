
#include "base/DocumentationStore.h"
#include "base/JsonSchema.h"

#include <iostream>
#include "json-schema.hpp"
#include "c-interfaces/Logging.h"

using namespace VnV;

DocumentationStore::DocumentationStore() {}

DocumentationStore& DocumentationStore::instance() {
  static DocumentationStore store;
  return store;
}

void DocumentationStore::addDocumentation(std::string packageName, std::string name, std::map<std::string, std::string> &map) {
  document_factory.insert(std::make_pair(packageName + name, map));
}

VnVParameterSet DocumentationStore::getParameterMap(std::string pname, std::string doc, std::map<std::string, std::pair<std::string, void *>> &m) {
     // Map the types.
     auto docs = document_factory.find(pname + doc);
     VnVParameterSet s;
     if (docs != document_factory.end()) {
        for (auto &it : m ) {
            auto ty = docs->second.find(it.first);
            if (ty != docs->second.end() ) {
                VnVParameter p(it.second.second, ty->second);
                p.setRtti(it.second.first);
                s.insert(std::make_pair(it.first,p));
            }
        }
     }
     return s;
}
using nlohmann::json_schema::json_validator;

void DocumentationStore::registerDocumentation(std::string packageName, std::string jsonStr) {

  json x = json::parse(jsonStr);
  json_validator validator;
  validator.set_root_schema(getDocumentationSchema());

  validator.validate(x);

  for (auto doc : x.items()) {
    json &x = doc.value();
    std::string name = x["name"].get<std::string>();
    std::map<std::string, std::string> m;
    if (x.contains("parameters")) {
      for ( auto &it : x["parameters"].items()) {
          m.insert(std::make_pair(it.key(),it.value().get<std::string>()));
      }
    }
    DocumentationStore::instance().addDocumentation(packageName, name, m);
  }
}
