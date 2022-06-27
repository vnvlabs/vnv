/**
  @file PlugStore.cpp Implementation of the injection point store as
defined in base/PlugStore.h.
**/
#include "base/stores/PlugStore.h"

#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"  // getplugDeclarationSchema
#include "base/points/PlugPoint.h"
#include "base/stores/PlugsStore.h"
#include "common-interfaces/Logging.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/IPlug.h"
#include "interfaces/ITest.h"  // TestConfig

using namespace VnV;

BaseStoreInstance(PlugStore)
BaseStoreInstance(PlugsStore)

    PlugStore::PlugStore() {}

std::shared_ptr<PlugPoint> PlugStore::newPlug(std::string packageName, std::string name, struct VnV_Function_Sig pretty,
                                              NTV& args) {
  std::string key = packageName + ":" + name;
  auto it = plugs.find(key);
  auto reg = registeredPlugs.find(key);

  if (it != plugs.end() && reg != registeredPlugs.end()) {
    FunctionSigniture sig(pretty);

    if (sig.run(it->second.runConfig)) {
      std::map<std::string, std::string> spec_map;
      bool foundOne;
      for (auto& it : reg->second.specJson.items()) {
        if (sig.match(it.key())) {
          for (auto itt : it.value().items()) {
            spec_map[itt.key()] = itt.value().get<std::string>();
          }
          foundOne = true;
          break;
        }
      }
      if (!foundOne) {
        json j = json::array();
        for (auto it : reg->second.specJson.items()) {
          j.push_back(it.key());
        }
        throw INJECTION_BUG_REPORT("No template specification matched: %s:%s:%s", pretty.signiture, pretty.compiler,
                                   j.dump());
      }

      // Construct and reset because InjectionPoint ctor is only accessible in
      // InjectionPointStore.
      std::shared_ptr<PlugPoint> injectionPoint;
      injectionPoint.reset(new PlugPoint(packageName, name, spec_map, args));
      for (auto& test : it->second.tests) {
        if (sig.match(test.getRunConfig())) {
          injectionPoint->addTest(test);
        }
      }

      if (it->second.plug != nullptr && sig.run(it->second.plug->getRunConfig())) {
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

bool PlugStore::registeredPlug(std::string package, std::string name) {
  return plugs.find(package + ":" + name) != plugs.end();
}

json PlugStore::schema(json& packageJson) {
  nlohmann::json temp = R"({
      "description": "An injection plug defined somewhere in the code",
      "type": "object",
      "properties": {
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
            "$ref": "#/definitions/test"
        },
        "plug" : {
            "$ref": "#/definitions/plugger"
        } 
      }
    })"_json;

  nlohmann::json props = json::object();
  for (auto& it : registeredPlugs) {
    json j = temp;
    j["description"] = packageJson[it.second.package]["InjectionPoints"][it.second.name]["docs"]["description"];
    j["vnvprops"] = it.second.specJson;
    props[it.second.package + ":" + it.second.name] = j;
  }
  json j = json::object();
  j["type"] = "object";
  j["properties"] = props;
  j["additionalProperties"] = false;
  return j;
}

void PlugStore::registerPlug(std::string packageName, std::string id,

                             std::string parameters_str) {
  VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(), id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerPlug(packageName, id, x);
  } catch (std::exception &e) {
    VnV_Warn(VNVPACKAGENAME, "Could not register Injection Point. Invalid Json. %s", parameters_str.c_str());
  }
}

std::shared_ptr<PlugPoint> PlugStore::getNewPlug(std::string package, std::string name, struct VnV_Function_Sig pretty,

                                                 NTV& args) {
  std::string key = package + ":" + name;
  if (plugs.find(key) == plugs.end()) {
    return nullptr;  // Not configured
  }
  return newPlug(package, name, pretty, args);
}

void PlugStore::addPlug(std::string package, std::string name, bool runInternal, json& templateName,
                        std::vector<TestConfig>& tests, std::shared_ptr<PlugConfig>& nPlugs) {
  std::string key = package + ":" + name;
  plugs.insert(std::make_pair(key, InjectionPlugConfig(package, name, runInternal, templateName, tests, nPlugs)));
}
