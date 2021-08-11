/**
  @file InjectionPointStore.cpp Implementation of the injection point store as
defined in base/InjectionPointStore.h.
**/

#include "base/stores/InjectionPointStore.h"

#include "base/points/InjectionPoint.h"   // InjectionPoint.
#include "base/parser/JsonSchema.h"       // getInjectionPointDeclarationSchema
#include "c-interfaces/Logging.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/ITest.h"      // TestConfig
#include "base/stores/SamplerStore.h"
#include "base/Runtime.h"

using namespace VnV;

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(
    std::string packageName, std::string name, NTV& in_args) {

    std::string key = packageName + ":" + name;
  auto it = injectionPoints.find(key);
  auto reg = registeredInjectionPoints.find(key);

  if (it != injectionPoints.end() && reg != registeredInjectionPoints.end()) {
    // Construct and reset because InjectionPoint ctor is only accessible in
    // InjectionPointStore.
    std::shared_ptr<InjectionPoint> injectionPoint;
    injectionPoint.reset(
        new InjectionPoint(packageName, name, reg->second.specJson, in_args));

    for (auto& test : it->second.tests) {
      injectionPoint->addTest(test);
    }
 
    injectionPoint->runInternal = it->second.runInternal;
    
    return injectionPoint;
  }
  return nullptr;
}

void InjectionPointStore::registerInjectionPoint(std::string packageName,
                                                 std::string id,
                                                 json& jsonObject) {
  
  
  registeredInjectionPoints.insert( std::make_pair(packageName + ":" + id, InjectionPointSpec(packageName,id,jsonObject)));
}

void InjectionPointStore::registerInjectionPoint(std::string packageName,
                                                 std::string id,
                                                 std::string parameters_str) {
 
 VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(),
            id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerInjectionPoint(packageName, id, x);
  } catch (...) {
    VnV_Warn(VNVPACKAGENAME,
             "Could not register Injection Point. Invalid Json. %s",
             parameters_str.c_str());
  }
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getNewInjectionPoint(
    std::string package, std::string name, InjectionPointType type, NTV& in_args) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  } else if (type == InjectionPointType::Single) {
    ptr = newInjectionPoint(package, name, in_args);
  } else if (type == InjectionPointType::Begin) { /* New staged injection point.
                                                  -- Add it to the stack */
    ptr = newInjectionPoint(package, name,   in_args); /*Not nullptr because we checked above*/
    registerLoopedInjectionPoint(package, name, ptr);
  } else {
    // Should never happen.
    return fetchFromQueue(package, name, type);
  }
  return ptr;
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getExistingInjectionPoint(
    std::string package, std::string name, InjectionPointType type) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  }
  return fetchFromQueue(package, name, type);
}

void InjectionPointStore::registerLoopedInjectionPoint(
    std::string package, std::string name,
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

std::shared_ptr<InjectionPoint> InjectionPointStore::fetchFromQueue(
    std::string package, std::string name, InjectionPointType stage) {
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


void InjectionPointStore::addInjectionPoint(std::string package,
                                            std::string name,
                                            bool runInternal,
                                            std::vector<TestConfig>&tests,
                                            const SamplerConfig& sinfo) {

 std::string key = package + ":" + name;

  auto reg = registeredInjectionPoints.find(key);
  json parameters = (reg == registeredInjectionPoints.end()) ? json::object() : (reg->second.specJson);

  // Build an empty parameter map for the InjectionPoint.
  std::map<std::string, std::string> parameterMap;  // maps injection point parameter to parameter type.
  for (auto it : parameters.items()) {
    parameterMap[StringUtils::squash_copy(it.key())] =
        StringUtils::squash_copy(it.value().get<std::string>());
  }

  tests.erase(
      std::remove_if(tests.begin(), tests.end(),
                     [&](TestConfig& t) {
                       if (t.preLoadParameterSet(parameterMap)) {
                         VnV_Debug(VNVPACKAGENAME, "Test Added Successfully %s",
                                   t.getName().c_str());
                         return false;
                       }
                       VnV_Warn(VNVPACKAGENAME, "Test Config is Invalid %s",
                                t.getName().c_str());
                       return true;
                     }),
  tests.end());

  if (!sinfo.name.empty() && ! SamplerStore::instance().createSampler(sinfo)) {
    VnV_Warn(VNVPACKAGENAME, "Sampler configuration is invalid");
  }

  injectionPoints.insert(std::make_pair(key, InjectionPointConfig(package,name,runInternal,tests)));
}

void InjectionPointStore::print() {
  for (auto it : registeredInjectionPoints) {
    VnV_Info(VNVPACKAGENAME, "%s(%s)", it.first.c_str(),
             it.second.specJson.dump().c_str());
  }
 
  for (auto it : injectionPoints) {
    VnV_Info("Iternal Run Configuration is %s",
             (it.second.runInternal) ? "On" : "Off");
  
   
  }
}

BaseStoreInstance(InjectionPointStore)
