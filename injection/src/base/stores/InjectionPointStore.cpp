﻿/**
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
                                                                       const char* pretty ,  
                                                                       NTV& in_args) {
  
  std::string key = packageName + ":" + name;
  auto it = injectionPoints.find(key);
  auto reg = registeredInjectionPoints.find(key);
  std::cout << "INJECTION POINT PRETTY " << pretty << std::endl;
  if (it != injectionPoints.end() && reg != registeredInjectionPoints.end()) {
     TemplateCallback templateCallback(pretty); 
     if (templateCallback.match(it->second.runTemplateName)) {

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
        std::shared_ptr<InjectionPoint> injectionPoint;
        injectionPoint.reset(new InjectionPoint(packageName, name, spec_map, in_args));
        for (auto& test : it->second.tests) {
          if (templateCallback.match(test.runTemplateName)) {  
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
                                                                         const char* pretty, 
                                                                          InjectionPointType type, NTV& in_args) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  } else if (type == InjectionPointType::Single) {
    ptr = newInjectionPoint(package, name, pretty, in_args);
  } else if (type == InjectionPointType::Begin) {    /* New staged injection point.
                                                     -- Add it to the stack */
    ptr = newInjectionPoint(package, name, pretty, in_args); /*Not nullptr because we checked above*/
    registerLoopedInjectionPoint(package, name, ptr);
  } else {
    // Should never happen.
    return fetchFromQueue(package, name, type);
  }
  return ptr;
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getExistingInjectionPoint(std::string package, std::string name,
                                                                               InjectionPointType type) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  }
  return fetchFromQueue(package, name, type);
}

void InjectionPointStore::registerLoopedInjectionPoint(std::string package, std::string name,
                                                       std::shared_ptr<InjectionPoint>& ptr) {
  std::string key = package + ":" + name;
  auto it = active.find(key);
  if (it == active.end()) {
    std::stack<std::shared_ptr<InjectionPoint>> s;
    s.push(ptr);
    active.insert(std::make_pair(key, s));
  } else {
    it->second.push(ptr);
  }
}

std::shared_ptr<InjectionPoint> InjectionPointStore::fetchFromQueue(std::string package, std::string name,
                                                                    InjectionPointType stage) {
  // Stage > 0 --> Fetch the queue.
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  auto it = active.find(key);
  if (it == active.end() || it->second.size() == 0) {
    return nullptr; /* queue doesn't exist or not active ip. */
  } else if (stage == InjectionPointType::End) {
    ptr = it->second.top();  // Final stage, remove it from the stack.
    it->second.pop();
  } else {
    ptr = it->second.top();  // Intermediate stage -> return top of stack.
  }
  return ptr;
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
