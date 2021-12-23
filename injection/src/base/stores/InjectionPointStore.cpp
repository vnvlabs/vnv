/**
  @file InjectionPointStore.cpp Implementation of the injection point store as
defined in base/InjectionPointStore.h.
**/

#include "base/stores/InjectionPointStore.h"

#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"      // getInjectionPointDeclarationSchema
#include "base/points/InjectionPoint.h"  // InjectionPoint.
#include "base/stores/SamplerStore.h"
#include "c-interfaces/Logging.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/ITest.h"      // TestConfig

using namespace VnV;

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(std::string packageName, std::string name,
                                                                       struct VnV_Function_Sig pretty, NTV& in_args) {
  std::string key = packageName + ":" + name;
  auto it = injectionPoints.find(key);
  auto reg = registeredInjectionPoints.find(key);
  if (it != injectionPoints.end() && reg != registeredInjectionPoints.end()) {
    FunctionSigniture sig(pretty);
    if (sig.run(it->second.runConfig)) {
      std::map<std::string, std::string> spec_map;
      bool foundOne;
      auto& smap = reg->second.specJson;

      for (auto& it : smap.items()) {
        if (smap.size() == 1 || sig.match(it.key())) {
          for (auto itt : it.value().items()) {
            spec_map[itt.key()] = itt.value().get<std::string>();
          }
          foundOne = true;
          break;
        }
      }
      if (!foundOne) {
        std::string dump = "";
        for (auto ita : reg->second.specJson.items()) {
          dump += "\n" + ita.key();
        }
        StringUtils::squash(dump);
        std::string s = StringUtils::squash_copy(pretty.signiture);
        VnV_Warn(VNVPACKAGENAME,
                 "Could not find a parameter set matching the function Signiture:\n%s\n"
                 "The options are:%s",
                 s.c_str(), dump.c_str());
        return nullptr;
      }

      // Construct and reset because InjectionPoint ctor is only accessible in
      // InjectionPointStore.
      std::shared_ptr<InjectionPoint> injectionPoint;
      injectionPoint.reset(new InjectionPoint(packageName, name, spec_map, in_args));
      for (auto& test : it->second.tests) {
        if (sig.run(test.getRunConfig())) {
          injectionPoint->addTest(test);
        }
      }
      injectionPoint->runInternal = it->second.runInternal;

      return injectionPoint;
    }
  }
  return nullptr;
}

void InjectionPointStore::registerInjectionPoint(std::string packageName, std::string id, json& jsonObject) {
  registeredInjectionPoints.insert(
      std::make_pair(packageName + ":" + id, InjectionPointSpec(packageName, id, jsonObject)));
}

json InjectionPointStore::schema() {
  nlohmann::json temp = R"({
      "description": "An injection Point defined somewhere in the code",
      "type": "object",
      "properties": {
        "name": {
          "const": "string"
        },
        "package" : {
          "const" : "string"
        },
        "runInternal": {
           "type" : "boolean"
        },
        "sampler" : {
           "$ref" : "#/definitions/sampler"
        },
        "runScope": {
          "$ref" : "#/definitions/runScope"
        },
        "tests": {
          "type": "array",
          "items": {
            "$ref": "#/definitions/test"
          }
        }
      },
      "required": [
        "name","package"
      ]
    })"_json;

  nlohmann::json oneof = json::array();
  for (auto& it : registeredInjectionPoints) {
    json j = temp;
    j["properties"]["name"]["const"] = it.second.name;
    j["properties"]["package"]["const"] = it.second.package;
    j["vnvprops"] = it.second.specJson;
    oneof.push_back(j);
  }
  nlohmann::json ret = json::object();
  if (oneof.size() > 0) {
    ret["oneOf"] = oneof;
    return ret;
  } else {
    return R"({"const" : false})"_json;
  }
}

void InjectionPointStore::registerInjectionPoint(std::string packageName, std::string id, std::string parameters_str) {
  VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(), id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerInjectionPoint(packageName, id, x);
  } catch (...) {
    VnV_Warn(VNVPACKAGENAME, "Could not register Injection Point. Invalid Json. %s", parameters_str.c_str());
  }
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getNewInjectionPoint(std::string package, std::string name,
                                                                          struct VnV_Function_Sig pretty,
                                                                          InjectionPointType type, NTV& in_args) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  } else if (type == InjectionPointType::Single) {
    ptr = newInjectionPoint(package, name, pretty, in_args);
  } else if (type == InjectionPointType::Begin) {            /* New staged injection point.
                                                             -- Add it to the stack */
    ptr = newInjectionPoint(package, name, pretty, in_args); /*Not nullptr because we checked above*/
    registerLoopedInjectionPoint(package, name, ptr);
  } else {
    // Should never happen.
    throw INJECTION_BUG_REPORT("New Injection point called but stage is not single or begin (%s)", key.c_str());
  }
  return ptr;
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getExistingInjectionPoint(std::string package, std::string name,
                                                                               InjectionPointType type) {
  std::string key = package + ":" + name;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  }
  return fetchFromQueue(package, name, type);
}

void InjectionPointStore::registerLoopedInjectionPoint(std::string package, std::string name,
                                                       std::shared_ptr<InjectionPoint>& ptr) {
  active.push(ptr);
}

std::shared_ptr<InjectionPoint> InjectionPointStore::fetchFromQueue(std::string package, std::string name,
                                                                    InjectionPointType stage) {
  // Stage > 0 --> Fetch the queue.
  std::string key = package + ":" + name;

  if (active.size() == 0) {
    throw INJECTION_BUG_REPORT("Fetch from Queue called with no Injection points int the stack %s:%s", package.c_str(), name.c_str());
  }
  auto ptr = active.top();
  if (key.compare(ptr->getPackage() + ":" + ptr->getName()) != 0) {
    throw INJECTION_EXCEPTION("Injection Point Nesting Error. Cannot call %s stage  inside a %s:%s", key,
                                ptr->getPackage().c_str(), ptr->getName().c_str());
  }

  if (stage == InjectionPointType::End) {
    active.pop();
  }
  return ptr;
}

bool InjectionPointStore::registered(std::string package, std::string name) {
  std::cout << "FFFF " << injectionPoints.size() << " " << package << " " << name << std::endl;
  return injectionPoints.find(package+":"+name) != injectionPoints.end();
}
bool InjectionPointStore::registeredTest(std::string package, std::string name) {
   for (auto it : injectionPoints) {
      for (auto t : it.second.tests) {
        if (t.getPackage().compare(package) ==0 && t.getName().compare(name) == 0 ) {
          return true;
        }
      } 
   }
   return false;
}


void InjectionPointStore::addInjectionPoint(std::string package, std::string name, bool runInternal, json& templateName,
                                            std::vector<TestConfig>& tests, const SamplerConfig& sinfo) {
  std::string key = package + ":" + name;

  if (!sinfo.name.empty() && !SamplerStore::instance().createSampler(sinfo)) {
    VnV_Warn(VNVPACKAGENAME, "Sampler configuration is invalid");
  }

  injectionPoints.insert(std::make_pair(key, InjectionPointConfig(package, name, runInternal, templateName, tests)));
}

void InjectionPointStore::print() {
  for (auto it : registeredInjectionPoints) {
    VnV_Info(VNVPACKAGENAME, "%s(%s)", it.first.c_str(), it.second.specJson.dump().c_str());
  }

  for (auto it : injectionPoints) {
    VnV_Info("Internal Run Configuration is %s", (it.second.runInternal) ? "On" : "Off");
  }
}

BaseStoreInstance(InjectionPointStore)
