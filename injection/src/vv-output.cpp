
#include <string>
#include <iostream>
#include "vv-output.h"

using namespace VnV;

void EngineStore::setEngineManager(std::string type, std::string outfileName, std::string configFile, bool debug) {
   
  auto it = registeredEngines.find(type);
  if ( it != registeredEngines.end() ) {
    manager = it->second(outfileName,configFile,debug);
    return;
  } 
  std::cerr << "Invalid Engine Name: " << type << std::endl; 
  throw "Invalid Engine Name" ;
}

OutputEngineManager* EngineStore::getEngineManager() {
  if ( manager != NULL ) 
    return manager;
  throw "Engine Not Initialized Error";
}

EngineStore::EngineStore(){}

EngineStore& EngineStore::getEngineStore() {
  static EngineStore *engine = new EngineStore();
  return *engine;
}

void EngineStore::registerEngine(std::string name, engine_register_ptr* engine_ptr) {
  registeredEngines.insert(std::make_pair(name,engine_ptr));
}

void VnV_registerEngine(std::string name, engine_register_ptr r) { 
  EngineStore::getEngineStore().registerEngine(name,r);  
}

IOutputEngine::~IOutputEngine(){}

