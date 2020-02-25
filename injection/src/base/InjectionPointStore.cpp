/**
  @file InjectionPointStore.cpp Implementation of the injection point store as defined in
  base/InjectionPointStore.h.
**/

#include <iostream>
#include "json-schema.hpp"
#include "base/Logger.h"
#include "base/OutputEngineStore.h"
#include "base/TestStore.h"
#include "base/JsonSchema.h"
#include "base/InjectionPointStore.h"
#include "c-interfaces/Injection.h"
#include "c-interfaces/Logging.h"

using namespace VnV;

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(
    std::string key, va_list args) {
  auto it = injectionPoints.find(key);
  if (it != injectionPoints.end()) {
    std::shared_ptr<InjectionPoint> injectionPoint =
        std::make_shared<InjectionPoint>(it->first, args);

    VnV_Debug("Adding the Tests to injection point %s " , it->first.c_str());
    for (auto& test : it->second) {
      injectionPoint->addTest(test);
    }
    return injectionPoint;
  }
  return nullptr;
}

void InjectionPointStore::registerInjectionPoint(json &jsonObject) {
    // Validate it against the schema for defining an injection point.
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(getInjectionPointDeclarationSchema());
    validator.validate(jsonObject);

    // Register the injection point in the store:(Validation throws)
    std::string name = jsonObject.find("name").value().get<std::string>();
    registeredInjectionPoints.insert(std::make_pair(name,jsonObject));

}

void InjectionPointStore::registerInjectionPoint(std::string json_str) {
    // Parse the json. We support a single injection point and an array of injection points.
    json x = json::parse(json_str);
    if (x.is_array()) {
        for ( auto it : x.items() ) {
            try {
                registerInjectionPoint(it.value());
            } catch (std::exception e) {
                VnV_Warn("Could not register Injection Point. Invalid Json. %s", it.value().dump().c_str());
            }
        }
    } else {
        registerInjectionPoint(x);
    }

}

json InjectionPointStore::getInjectionPointRegistrationJson(std::string name) {
    auto it = registeredInjectionPoints.find(name);
    if ( it != registeredInjectionPoints.end()) {
        return it->second;
    }
    return json(nullptr);
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getInjectionPoint(
    std::string key, InjectionPointType stage, va_list args) {


  //VnV_Debug("Looking for injection point {} at stage {} ", key, stage );
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    VnV_Debug("Injection point %s not configured ", key.c_str() );
    return nullptr;  // Not configured
  } else if (stage == InjectionPointType::Single) {
    ptr = newInjectionPoint(key, args);
  } else if (stage == InjectionPointType::Begin) { /* New staged injection point. -- Add it to the stack */
    auto it = active.find(key);
    ptr = newInjectionPoint(key, args); /*Not nullptr because we checked above*/

    /* If first of its kind, make a new map entry. */
    if (it == active.end()) {
      std::stack<std::shared_ptr<InjectionPoint>> s;
      s.push(ptr);
      active.insert(std::make_pair(key, s));
    } else {
      it->second.push(ptr);
    }
  } else {
    // Stage > 0 --> Fetch the queue.
    auto it = active.find(key);
    if (it == active.end() || it->second.size() == 0) {
      return nullptr; /* queue doesn't exist or not active ip. */
    } else if (stage == InjectionPointType::End) {
      ptr = it->second.top();  // Final stage, remove it from the stack.
      it->second.pop();
    } else {
      ptr = it->second.top();  // Intermediate stage -> return top of stack.
    }
  }
  return ptr;
}

InjectionPointStore& InjectionPointStore::getInjectionPointStore() {
  static InjectionPointStore store;
  return store;
}

void InjectionPointStore::addInjectionPoint(std::string name,
                                            std::vector<TestConfig>& tests) {
    auto reg = registeredInjectionPoints.find(name);
    if ( reg!=registeredInjectionPoints.end()) {
        VnV_Warn("Injection Point %s Is Registered, but Injection point to Test Mapping "

                    "has not been validated yet. The injection point to test mapping will"
                    "be un validated", name.c_str());
    } else {
        for ( auto it : registeredInjectionPoints ) {
            printf("%s %s ", name.c_str(), it.first.c_str());
        }
        VnV_Warn("The injection point %s has not been registered with the Injection point store. All calls"
                 "to this injection point will be validated at runtime.", name.c_str());
    }
    injectionPoints.insert(std::make_pair(name, tests));
}

void InjectionPointStore::addInjectionPoints(
    std::map<std::string, std::vector<TestConfig>>& injectionPoints) {
    for (auto it : injectionPoints) {
        addInjectionPoint(it.first,it.second);
    }

    this->injectionPoints.insert(injectionPoints.begin(), injectionPoints.end());
}

void InjectionPointStore::print() {
    auto rip = VnV_BeginStage("Registered Injection Points");
    for (auto it : registeredInjectionPoints) {
        VnV_Info("%s: %s", it.first.c_str(), it.second.dump().c_str());
    }
    VnV_EndStage(rip);

    int ups = VnV_BeginStage("InjectionPointStore Configuration");
    for ( auto it: injectionPoints) {
        int s = VnV_BeginStage("Name: %s", it.first.c_str());
        for ( auto itt : it.second ) {
            auto ss = VnV_BeginStage("Test %s:", itt.getName().c_str());
            itt.print();
            VnV_EndStage(ss);
        }
        VnV_EndStage(s);
   }
   VnV_EndStage(ups);
}
