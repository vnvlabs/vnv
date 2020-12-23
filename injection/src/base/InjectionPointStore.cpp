/**
  @file InjectionPointStore.cpp Implementation of the injection point store as
defined in base/InjectionPointStore.h.
**/

#include "base/InjectionPointStore.h"

#include "base/InjectionPoint.h"   // InjectionPoint.
#include "base/JsonSchema.h"       // getInjectionPointDeclarationSchema
#include "c-interfaces/Logging.h"  //Logging Statements (VnV_Debug, etc)
#include "interfaces/ITest.h"      // TestConfig

using namespace VnV;

InjectionPointStore::InjectionPointStore() {}

std::shared_ptr<InjectionPoint> InjectionPointStore::newInjectionPoint(
    std::string packageName, std::string name, NTV& in_args, NTV& out_args) {

    std::string key = packageName + ":" + name;
  auto it = injectionPoints.find(key);
  auto reg = registeredInjectionPoints.find(key);

  if (it != injectionPoints.end() && reg != registeredInjectionPoints.end()) {
    // Construct and reset because InjectionPoint ctor is only accessible in
    // InjectionPointStore.
    std::shared_ptr<InjectionPoint> injectionPoint;
    injectionPoint.reset(
        new InjectionPoint(packageName, name, reg->second.specJson, reg->second.iterator, in_args, out_args));

    for (auto& test : it->second.tests) {
      injectionPoint->addTest(test);
    }
    for (auto&test: it->second.iterators) {
        injectionPoint->addIterator(test);
    }
    injectionPoint->runInternal = it->second.runInternal;
    return injectionPoint;
  }
  return nullptr;
}

void InjectionPointStore::registerInjectionPoint(std::string packageName,
                                                 std::string id,
                                                 bool iterative,
                                                 json& jsonObject) {
  // Register the injection point in the store:(Validation throws)
  VnV_Debug(VNVPACKAGENAME, "NAME: %s:%s", packageName.c_str(), id.c_str());
  registeredInjectionPoints.insert(
      std::make_pair(packageName + ":" + id, InjectionPointSpec(packageName,id,jsonObject,iterative)));
}

void InjectionPointStore::registerInjectionPoint(std::string packageName,
                                                 std::string id,
                                                 bool iterative,
                                                 std::string parameters_str) {
  VnV_Debug(VNVPACKAGENAME, "Registering %s:%s ", packageName.c_str(),
            id.c_str());
  // Parse the json. We support a single injection point and an array of
  // injection points.
  try {
    json x = json::parse(parameters_str);
    registerInjectionPoint(packageName, id, iterative, x);
  } catch (...) {
    VnV_Warn(VNVPACKAGENAME,
             "Could not register Injection Point. Invalid Json. %s",
             parameters_str.c_str());
  }
}

std::shared_ptr<InjectionPoint> InjectionPointStore::getNewInjectionPoint(
    std::string package, std::string name, InjectionPointType type, NTV& in_args, NTV& out_args) {
  std::string key = package + ":" + name;
  std::shared_ptr<InjectionPoint> ptr;
  if (injectionPoints.find(key) == injectionPoints.end()) {
    return nullptr;  // Not configured
  } else if (type == InjectionPointType::Single) {
    ptr = newInjectionPoint(package, name, in_args,out_args);
  } else if (type == InjectionPointType::Begin) { /* New staged injection point.
                                                  -- Add it to the stack */
    ptr = newInjectionPoint(package, name,
                            in_args,out_args); /*Not nullptr because we checked above*/
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

InjectionPointStore& InjectionPointStore::getInjectionPointStore() {
  static InjectionPointStore store;
  return store;
}

void InjectionPointStore::addInjectionPoint(std::string package,
                                            std::string name,
                                            bool runInternal,
                                            std::vector<TestConfig>&tests,
                                            std::vector<TestConfig> &iterators) {

 std::string key = package + ":" + name;

  auto reg = registeredInjectionPoints.find(key);
  json parameters = (reg == registeredInjectionPoints.end()) ? json::object() : (reg->second.specJson);

  // make it an iterator if not unregistered. Warn user that there is no gaurantee that iterators
  // applied to unregistered injection points will be executed
  bool iterator = (reg == registeredInjectionPoints.end()) ? true : (reg->second.iterator);

  // Build an empty parameter map for the InjectionPoint.
  std::map<std::string, std::string> parameterMap;  // maps injection point parameter to parameter type.
  for (auto it : parameters.items()) {
    parameterMap[StringUtils::squash_copy(it.key())] =
        StringUtils::squash_copy(it.value().get<std::string>());
  }

  tests.erase(
      std::remove_if(tests.begin(), tests.end(),
                     [&](TestConfig& t) {
                       if (t.isIterator()) {
                          VnV_Warn(VNVPACKAGENAME, "Iterator cant be used as test %s",
                                t.getName().c_str());
                            return true;
                       }
                       else if (t.preLoadParameterSet(parameterMap)) {
                         VnV_Debug(VNVPACKAGENAME, "Test Added Successfully %s",
                                   t.getName().c_str());
                         return false;
                       }
                       VnV_Warn(VNVPACKAGENAME, "Test Config is Invalid %s",
                                t.getName().c_str());
                       return true;
                     }),
      tests.end());


    iterators.erase(
      std::remove_if(iterators.begin(), iterators.end(),
                     [&](TestConfig& t) {
                        if (!iterator) {
                            VnV_Warn(VNVPACKAGENAME, "Iterators cant be added to standard injection poitns %s",
                            t.getName().c_str());
                            return true;
                       } else if (!t.isIterator()) {
                          VnV_Warn(VNVPACKAGENAME, "Tesst cant be used as iterator %s",
                                t.getName().c_str());
                            return true;
                       }
                       else if (t.preLoadParameterSet(parameterMap)) {
                         VnV_Debug(VNVPACKAGENAME, "Iterator Added Successfully %s",
                                   t.getName().c_str());
                         return false;
                       }
                       VnV_Warn(VNVPACKAGENAME, "Iterator Config is Invalid %s",
                                t.getName().c_str());
                       return true;
                     }),
      iterators.end());

  injectionPoints.insert(std::make_pair(key, InjectionPointConfig(package,name,runInternal,tests,iterators)));
}

void InjectionPointStore::print() {
  auto rip = VnV_BeginStage(VNVPACKAGENAME, "Registered Injection Points");
  for (auto it : registeredInjectionPoints) {
    VnV_Info(VNVPACKAGENAME, "%s(%s)", it.first.c_str(),
             it.second.specJson.dump().c_str());
  }
  VnV_EndStage(VNVPACKAGENAME, rip);

  int ups = VnV_BeginStage(VNVPACKAGENAME, "InjectionPointStore Configuration");
  for (auto it : injectionPoints) {
    int s = VnV_BeginStage(VNVPACKAGENAME, "Key: %s", it.first.c_str());
    VnV_Info("Iternal Run Configuration is %s",
             (it.second.runInternal) ? "On" : "Off");
    for (auto itt : it.second.tests) {
      auto ss =
          VnV_BeginStage(VNVPACKAGENAME, "Test %s:", itt.getName().c_str());
      itt.print();
      VnV_EndStage(VNVPACKAGENAME, ss);
    }
    for (auto itt : it.second.iterators) {
      auto ss =
          VnV_BeginStage(VNVPACKAGENAME, "Test %s:", itt.getName().c_str());
      itt.print();
      VnV_EndStage(VNVPACKAGENAME, ss);
    }
    VnV_EndStage(VNVPACKAGENAME, s);
  }
  VnV_EndStage(VNVPACKAGENAME, ups);
}
