/**
  @file IteratorStore.cpp Implementation of the injection point store as
defined in base/IteratorStore.h.
**/

#include "base/stores/IteratorStore.h"

#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"  // getInjectionPointDeclarationSchema
#include "base/points/IteratorPoint.h"
#include "base/stores/IteratorsStore.h"
#include "c-interfaces/Logging.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/IIterator.h"
#include "interfaces/ITest.h"  // TestConfig

using namespace VnV;

IteratorStore::IteratorStore() {}

std::shared_ptr<IterationPoint> IteratorStore::newIterator(std::string packageName, std::string name,
                                                           struct VnV_Function_Sig pretty, int once_, NTV& in_args,
                                                           NTV& out_args) {
  std::string key = packageName + ":" + name;
  auto it = iterators.find(key);
  auto reg = registeredIterators.find(key);

  if (it != iterators.end() && reg != registeredIterators.end()) {
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
      std::shared_ptr<IterationPoint> injectionPoint;
      injectionPoint.reset(new IterationPoint(packageName, name, spec_map, once_, in_args, out_args));
      for (auto& test : it->second.tests) {
        if (sig.match(test.getRunConfig())) {
          injectionPoint->addTest(test);
        }
      }

      for (auto& test : it->second.iterators) {
        if (sig.run(test.getRunConfig())) {
          injectionPoint->addIterator(test);
        }
      }

      injectionPoint->runInternal = it->second.runInternal;

      return injectionPoint;
    }
  }
  return nullptr;
}

void IteratorStore::registerIterator(std::string packageName, std::string id, json& jsonObject) {
  registeredIterators.insert(std::make_pair(packageName + ":" + id, InjectionPointSpec(packageName, id, jsonObject)));
}

void IteratorStore::registerIterator(std::string packageName, std::string id,

                                     std::string parameters_str) {
  VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(), id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerIterator(packageName, id, x);
  } catch (...) {
    VnV_Warn(VNVPACKAGENAME, "Could not register Injection Point. Invalid Json. %s", parameters_str.c_str());
  }
}

json IteratorStore::schema() {
  nlohmann::json temp = R"({
      "description": "An injection iterator defined somewhere in the code",
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
          "$ref" : "#/definitions/test"
        },
        "iterators": {
          "$ref" : "#/definitions/iterators"
        } 
      }
    })"_json;

  nlohmann::json props = json::object();
  for (auto& it : registeredIterators) {
    json j = temp;
    j["vnvprops"] = it.second.specJson;
    props[it.second.package + ":" + it.second.name] = j;
  }
  json j = json::object();
  j["type"] = "object";
  j["properties"] = props;
  j["additionalProperties"] = false;
  return j;
}

std::shared_ptr<IterationPoint> IteratorStore::getNewIterator(std::string package, std::string name,
                                                              struct VnV_Function_Sig pretty, int once, NTV& in_args,
                                                              NTV& out_args) {
  std::string key = package + ":" + name;
  if (iterators.find(key) == iterators.end()) {
    return nullptr;  // Not configured
  }

  return newIterator(package, name, pretty, once, in_args, out_args);
}

void IteratorStore::addIterator(std::string package, std::string name, bool runInternal, json& templateName,
                                std::vector<TestConfig>& tests, std::vector<IteratorConfig>& niterators) {
  std::string key = package + ":" + name;
  iterators.insert(
      std::make_pair(key, InjectionIteratorConfig(package, name, runInternal, templateName, tests, niterators)));
}

bool IteratorStore::registeredIterator(std::string package, std::string name) {
  return iterators.find(package + ":" + name) != iterators.end();
}

BaseStoreInstance(IteratorStore) BaseStoreInstance(IteratorsStore)
