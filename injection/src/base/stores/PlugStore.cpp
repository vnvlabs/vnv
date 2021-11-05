/**
  @file PlugStore.cpp Implementation of the injection point store as
defined in base/PlugStore.h.
**/
#include "base/stores/PlugStore.h"

#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"  // getplugDeclarationSchema
#include "base/points/PlugPoint.h"
#include "base/stores/PlugsStore.h"
#include "c-interfaces/Logging.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/IPlug.h"
#include "interfaces/ITest.h"  // TestConfig

using namespace VnV;

BaseStoreInstance(PlugStore) BaseStoreInstance(PlugsStore)

    PlugStore::PlugStore() {}

std::shared_ptr<PlugPoint> PlugStore::newPlug(std::string packageName, std::string name,
                                              const VnV::TemplateCallback& templateCallback, NTV& in_args,
                                              NTV& out_args) {
  std::string key = packageName + ":" + name;
  auto it = plugs.find(key);
  auto reg = registeredPlugs.find(key);


  if (it != plugs.end() && reg != registeredPlugs.end()) {
    
     auto templateName = templateCallback.m; 
     if (it->second.runTemplate(templateName)) {

        std::map<std::string,std::string> spec_map;
        bool foundOne;
        for (auto &it : reg->second.specJson.items()) {
          json& template_spec = it.value()["templates"];
          if (templateCallback.match(template_spec)) {
              for (auto itt : it.value()["parameters"].items()) {
                spec_map[itt.key()] = itt.value().get<std::string>();
              }
              foundOne = true;
              break;
          }
        }
        if (!foundOne) {
          throw VnVExceptionBase("No template specification matched -- Bugs... run");
        }
      
        // Construct and reset because InjectionPoint ctor is only accessible in
        // InjectionPointStore.
        std::shared_ptr<PlugPoint> injectionPoint;
        injectionPoint.reset(new PlugPoint(packageName, name, spec_map, in_args, out_args));
        for (auto& test : it->second.tests) {
          if (test.runTemplate(templateName)) {  
              injectionPoint->addTest(test);
          }
        }

        if (it->second.plug != nullptr && it->second.plug->runTemplate(templateName) ) {
          injectionPoint->setPlug(*it->second.plug);
        }

        injectionPoint->runInternal = it->second.runInternal;

        return injectionPoint;
     }
  }
  return nullptr;
}

void PlugStore::registerPlug(std::string packageName, std::string id, json& jsonObject) {
  registeredPlugs.insert(std::make_pair(packageName + ":" + id, PlugSpec(packageName, id, jsonObject)));
}

json PlugStore::schema() {
  nlohmann::json temp = R"({
      "description": "An injection plug defined somewhere in the code",
      "type": "object",
      "properties": {
        "name": {
          "type": "string"
        },
        "package" : {
          "type" : "string"
        },
        "runInternal": {
           "type" : "boolean"
        },
        "runScope": {
          "type": "array",
          "items": {
            "type": "string"
          }
        },
        "tests": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/test"
          }
        },
        "plug" : {
            "$ref": "#/definitions/plugger"
        } 
      },
      "required": [
        "name","package"
      ]
    })"_json;

  nlohmann::json oneof = json::array();
  for (auto& it : registeredPlugs) {
    json j = temp;
    j["properties"]["name"]["const"] = it.second.name;
    j["properties"]["package"]["const"] = it.second.package;
    j["vnvprops"] = it.second.specJson;
    oneof.push_back(j);
  }

  if (oneof.size() > 0) {
    nlohmann::json ret = json::object();
    ret["oneOf"] = oneof;
    return ret;
  } else {
    return R"({"const" : false})"_json;
  }
}

void PlugStore::registerPlug(std::string packageName, std::string id,

                             std::string parameters_str) {
  VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(), id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerPlug(packageName, id, x);
  } catch (...) {
    VnV_Warn(VNVPACKAGENAME, "Could not register Injection Point. Invalid Json. %s", parameters_str.c_str());
  }
}

std::shared_ptr<PlugPoint> PlugStore::getNewPlug(std::string package, std::string name,
                                                 const VnV::TemplateCallback& templateCallback,

                                                 NTV& in_args, NTV& out_args) {
  std::string key = package + ":" + name;
  if (plugs.find(key) == plugs.end()) {
    return nullptr;  // Not configured
  }
  return newPlug(package, name, templateCallback, in_args, out_args);
}

void PlugStore::addPlug(std::string package, std::string name, bool runInternal, json& templateName, std::vector<TestConfig>& tests,
                        std::shared_ptr<PlugConfig>& nPlugs) {
  
  std::string key = package + ":" + name;
  plugs.insert(std::make_pair(key, InjectionPlugConfig(package, name, runInternal, templateName, tests, nPlugs)));

}
