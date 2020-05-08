/**
  @file InjectionPointStore.cpp Implementation of the injection point store as defined in
  base/InjectionPointStore.h.
**/

#include "base/InjectionPointStore.h"

#include "base/JsonSchema.h"   // getInjectionPointDeclarationSchema
#include "c-interfaces/Logging.h" //Logging Statements (VnV_Debug, etc)
#include "interfaces/ITest.h"     // TestConfig
#include "base/InjectionPoint.h"   // InjectionPoint.

using namespace VnV;

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(
    std::string key, NTV &args) {


  auto it = injectionPoints.find(key);
  auto reg = registeredInjectionPoints.find(key);

  if (it != injectionPoints.end() && reg != registeredInjectionPoints.end()) {

    // Construct and reset because InjectionPoint ctor is only accessible in InjectionPointStore.
    std::shared_ptr<InjectionPoint> injectionPoint;
    injectionPoint.reset( new InjectionPoint(reg->second, args));

    for (auto& test : it->second.second) {
      injectionPoint->addTest(test);
    }
    injectionPoint->runInternal = it->second.first;
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
    std::string package = jsonObject.find("package").value().get<std::string>();
    VnV_Debug("NAME: %s:%s", name.c_str(),package.c_str());
    registeredInjectionPoints.insert(std::make_pair(package + ":" + name,jsonObject));

}

void InjectionPointStore::registerInjectionPoint(std::string json_str) {

    VnV_Debug("Registering %s " , json_str.c_str());
    // Parse the json. We support a single injection point and an array of injection points.
    try {
        json x = json::parse(json_str);
        if (x.is_array()) {
            for ( auto it : x.items() ) {
                 try {
                    registerInjectionPoint(it.value());
                } catch (...) {
                    VnV_Warn("Could not register Injection Point. Invalid Json. %s", it.value().dump().c_str());
                }
            }
        } else {
            try {
                registerInjectionPoint(x);
            } catch(...) {
                 VnV_Warn("Could not register Injection Point. Invalid Json. %s", x.dump().c_str());

            }
        }
    } catch(...) {
        VnV_Warn("Could not register Injection Point. Invalid Json. %s", json_str.c_str());
    }
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getNewInjectionPoint(std::string package, std::string name, InjectionPointType type, NTV &args) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  } else if (type == InjectionPointType::Single) {
    ptr = newInjectionPoint(key, args);
  } else if (type == InjectionPointType::Begin) { /* New staged injection point. -- Add it to the stack */
    ptr = newInjectionPoint(key, args); /*Not nullptr because we checked above*/
    registerLoopedInjectionPoint(key,ptr);
  } else {
      //Should never happen.
     return fetchFromQueue(key, type);
  }
  return ptr; 
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getExistingInjectionPoint(std::string package, std::string name, InjectionPointType type) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  }
  return fetchFromQueue(key, type);
}




void InjectionPointStore::registerLoopedInjectionPoint(std::string key, std::shared_ptr<InjectionPoint>&ptr) {
     auto it = active.find(key);
     if (it == active.end()) {       
      std::stack<std::shared_ptr<InjectionPoint>> s;
      s.push(ptr);
      active.insert(std::make_pair(key, s));
    } else {
      it->second.push(ptr);
    }
    
}

std::shared_ptr<InjectionPoint> InjectionPointStore::fetchFromQueue(std::string key, InjectionPointType stage) {
    // Stage > 0 --> Fetch the queue.
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

InjectionPointStore& InjectionPointStore::getInjectionPointStore() {
  static InjectionPointStore store;
  return store;
}

void InjectionPointStore::addInjectionPoint(std::string package, std::string name,
                                            std::pair<bool,std::vector<TestConfig>>& tests) {
    std::string key = package + ":" + name;
    auto reg = registeredInjectionPoints.find(key);
    if ( reg!=registeredInjectionPoints.end()) {

        // Validate the test configs against the registered Json.
        json regJson = reg->second;
        json parameters = regJson["parameters"];

        // Build an empty parameter map for the InjectionPoint.
        std::map<std::string,std::string> parameterMap;
        for ( auto it : parameters.items()) {
            parameterMap[it.key()] = it.value().get<std::string>();
        }

        tests.second.erase(std::remove_if(tests.second.begin(), tests.second.end(), [&](TestConfig &t){
           if (t.preLoadParameterSet(parameterMap)) {
             VnV_Debug("Test Added Successfully %s" , t.getName().c_str());
             return false;
           }
           VnV_Warn("Test Config is Invalid %s", t.getName().c_str());
           return true;
        }), tests.second.end());

     } else {
        VnV_Warn("The injection point %s has not been registered with the Injection point store", key.c_str());
    }
    injectionPoints.insert(std::make_pair(key, tests));
}

void InjectionPointStore::print() {
    auto rip = VnV_BeginStage("Registered Injection Points");
    for (auto it : registeredInjectionPoints) {
        VnV_Info("%s: %s", it.first.c_str(), it.second.dump().c_str());
    }
    VnV_EndStage(rip);

    int ups = VnV_BeginStage("InjectionPointStore Configuration");
    for ( auto it: injectionPoints) {
        int s = VnV_BeginStage("Key: %s", it.first.c_str());
        VnV_Info("Iternal Run Configuration is %s", (it.second.first) ? "On" : "Off" );
        for ( auto itt : it.second.second ) {
            auto ss = VnV_BeginStage("Test %s:", itt.getName().c_str());
            itt.print();
            VnV_EndStage(ss);
        }
        VnV_EndStage(s);
   }
   VnV_EndStage(ups);
}
