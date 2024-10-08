﻿/**
  @file InjectionPointStore.cpp Implementation of the injection point store as
defined in base/InjectionPointStore.h.
**/

#include "base/stores/InjectionPointStore.h"

#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"      // getInjectionPointDeclarationSchema
#include "base/InjectionPoint.h"  // InjectionPoint.
#include "base/stores/SamplerStore.h"
#include "common-interfaces/all.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/ITest.h"           // TestConfig
#include "shared/exceptions.h"

using namespace VnV;

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(std::string packageName, std::string name,
                                                                       struct VnV_Function_Sig pretty, NTV& args) {
  std::string key = packageName + ":" + name;
  auto it = injectionPoints.find(key);
  auto reg = registeredInjectionPoints.find(key);
  if (it != injectionPoints.end() && reg != registeredInjectionPoints.end()) {
    FunctionSigniture sig(pretty);
    if (sig.run(it->second.runConfig)) {
      std::map<std::string, std::string> spec_map;
      bool foundOne = false;
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
      injectionPoint.reset(new InjectionPoint(packageName, name, spec_map, args));
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

json InjectionPointStore::schema(json& packageJson) {
  nlohmann::json temp = R"({
      "description": "An injection Point defined somewhere in the code",
      "type": "object",
      "properties": {
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
          "$ref" : "#/definitions/test"
        }
      }
    })"_json;

  nlohmann::json ipo = R"({
    "type" : "object",
    "additionalProperties" : false
  })"_json;

  json props = json::object();
  props["runAll"] = R"({"type" : "boolean", "default" : false, "description" : "Run all injection points"})"_json;
  props["runAll_tests"] = R"({"$ref" : "#/definitions/test"})"_json;
  
  for (auto& it : registeredInjectionPoints) {
    json j = temp;
    j["description"] = packageJson[it.second.package]["InjectionPoints"][it.second.name]["docs"]["description"];
    j["vnvprops"] = it.second.specJson;
    j["vnvparam"] = packageJson[it.second.package]["InjectionPoints"][it.second.name]["docs"]["params"];
    props[it.second.package + ":" + it.second.name] = j;
  }
  ipo["properties"] = props;
  return ipo;
}

void InjectionPointStore::registerInjectionPoint(std::string packageName, std::string id, std::string parameters_str) {
  VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(), id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerInjectionPoint(packageName, id, x);
  } catch (std::exception& e) {
    VnV_Warn(VNVPACKAGENAME, "Could not register Injection Point. Invalid Json. %s", parameters_str.c_str());
  }
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getNewInjectionPoint(std::string package, std::string name,
                                                                          struct VnV_Function_Sig pretty,
                                                                          InjectionPointType type, NTV& args) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  } else if (type == InjectionPointType::Single) {
    ptr = newInjectionPoint(package, name, pretty, args);
  } else if (type == InjectionPointType::Begin) {         /* New staged injection point.
                                                          -- Add it to the stack */
    ptr = newInjectionPoint(package, name, pretty, args); /*Not nullptr because we checked above*/
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
  active.push_back(ptr);
}


//We want to return a list of injection points that are open parents of this injection point
//For simplicity, a parent injection point is an open injection point with the same communicator
//as the current injection point. (In reality, we could do all injection points with comms that 
//are contained inside the current injection point, but we will get there eventually
std::vector<std::shared_ptr<InjectionPoint>> InjectionPointStore::getParents(InjectionPoint& ip, bool onqueue) {
    std::vector<std::shared_ptr<InjectionPoint>> res;
    bool found_me = !onqueue;
    for (auto it = active.rbegin(); it != active.rend(); it++) {
        if (! found_me ) {
          if ((*it).get() == &ip) {
            found_me = true;
          }
        } else {
        
          if ( (!(*it)->skipped) && (*it)->comm->uniqueId() == ip.comm->uniqueId()) {
              res.push_back(*it);
          }
        }
    }
    return res;
}


std::shared_ptr<InjectionPoint> InjectionPointStore::fetchFromQueue(std::string package, std::string name,
                                                                    InjectionPointType stage) {
  // Stage > 0 --> Fetch the queue.
  std::string key = package + ":" + name;

  if (active.size() == 0) {
    throw INJECTION_BUG_REPORT("Fetch from Queue called with no Injection points started %s:%s", package.c_str(),  name.c_str());
  }

  if (stage == InjectionPointType::End) {
      std::shared_ptr<InjectionPoint> ptr = active.back();
      if (key.compare(ptr->getPackage() + ":" + ptr->getName()) != 0) {
        throw INJECTION_EXCEPTION("Injection Point Nesting Error. Cannot End %s:%s as its not the last injection point to be started ",  package.c_str(), name.c_str());
      }
      active.pop_back();
      return ptr;
  } else if (stage == InjectionPointType::Iter) {
    for (auto r=active.rbegin(); r != active.rend(); r++) {
      if (key.compare((*r)->getPackage() + ":" + (*r)->getName()) == 0) {
        return *r;  
      }
    }
  } else {
      throw INJECTION_EXCEPTION("Tried to fetch with a non iterative point: %s:%s",  package.c_str(), name.c_str());
  }

  return nullptr;
}

bool InjectionPointStore::registered(std::string package, std::string name) {
  return injectionPoints.find(package + ":" + name) != injectionPoints.end();
}
bool InjectionPointStore::registeredTest(std::string package, std::string name) {
  for (auto it : injectionPoints) {
    for (auto t : it.second.tests) {
      if (t.getPackage().compare(package) == 0 && t.getName().compare(name) == 0) {
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

// Turn on all not configured injection points with default options.
void InjectionPointStore::runAll(std::vector<TestConfig>& tests) {
  for (auto& it : registeredInjectionPoints) {
    auto a = injectionPoints.find(it.first);
    if (a == injectionPoints.end()) {
      InjectionPointConfig c(it.second.package, it.second.name, true, json::object(), tests);
      injectionPoints.insert(std::make_pair(it.first, c));
    }
  }
}

void InjectionPointStore::print() {
  
  std::ostringstream oss;
  oss << "%sThe Following Injection points will run:\n";
  for (auto it : injectionPoints) {
    oss << "\t" << it.second.packageName <<":"<< it.second.name << " with tests\n";
    if (it.second.runInternal) {
         oss << "\t\t" << "Internal Test\n";
    }
    for (auto itt : it.second.tests) {
       oss << "\t\t" << itt.getPackage() << ":" << itt.getName() << "\n";
    }
  }
  std::string os = oss.str();
  VnV_Info(VNVPACKAGENAME, os.c_str(),"");

  //for (auto it : registeredInjectionPoints) {
  //  VnV_Info(VNVPACKAGENAME, "%s(%s)", it.first.c_str(), it.second.specJson.dump().c_str());
  //}


}

BaseStoreInstance(InjectionPointStore)
