#include <iostream>

#include "vv-output.h"
#include "vv-debug-engine.h"

using namespace VnV;

DebugEngine::DebugEngine() {}

void DebugEngine::Put(std::string variableName, double &value) {
  std::cout << "DEBUG ENGINE: Put: " << variableName << " = " << value << std::endl; 
}   
void DebugEngine::Put(std::string variableName, int &value) {
  std::cout << "DEBUG ENGINE: Put: " << variableName << " = " << value << std::endl; 
}   
void DebugEngine::Put(std::string variableName, float &value) {
  std::cout << "DEBUG ENGINE: Put: " << variableName << " = " << value << std::endl; 
}   
void DebugEngine::Put(std::string variableName, long &value) {
  std::cout << "DEBUG ENGINE: Put: " << variableName << " = " << value << std::endl;; 
}   

void DebugEngine::DefineDouble(std::string variableName) {
    std::cout << "DEBUG ENGINE: Define: " << variableName << " = double" << std::endl;  ; 
}

void DebugEngine::DefineFloat(std::string variableName) {
    std::cout << "DEBUG ENGINE: Define: " << variableName << " = float" <<  std::endl;; 
}

void DebugEngine::DefineInt(std::string variableName) { 
    std::cout << "DEBUG ENGINE: Define: " << variableName << " = int" <<  std::endl;; 
}


void DebugEngine::DefineLong(std::string variableName) {
    std::cout << "DEBUG ENGINE: Define: " << variableName << " = long" << std::endl;  ; 
}



DebugEngineWrapper::DebugEngineWrapper(std::string outfile, bool debug ) {
    set(outfile);
}

DebugEngineWrapper::DebugEngineWrapper(std::string outfile, std::string configFile,  bool debug ) {
    set(outfile);
}


void DebugEngineWrapper::finalize() {
  std::cout << "DEBUG ENGINE: Finalize Called on Debug Engine -- GoodBye. " << std::endl;
}

void DebugEngineWrapper::set(std::string outfile) {
  std::cout << "DEBUG ENGINE: Using debug Engine --> outfile : " << outfile  <<std::endl;
  debugEngine = new DebugEngine();
}

void DebugEngineWrapper::endInjectionPoint(std::string id, int stageVal ) {

    if ( debugEngine ) {
        std::cout << "DEBUG ENGINE: End of InjectionPoint : " << id << " : " << stageVal << std::endl;
    } else {
        throw "Engine not initialized" ;
    }
}

void DebugEngineWrapper::startInjectionPoint(std::string id, int stageVal  ) {
    if ( debugEngine ) {
        std::cout << "DEBUG ENGINE: Start of InjectionPoint : " << id << " : " << stageVal << std::endl;
    } else {
        throw "Engine not initialized" ;
    }
}

void DebugEngineWrapper::startTest( std::string testName, int testStageVal) {
    if ( debugEngine ) {
        std::cout << "DEBUG ENGINE: Start of Test : " << testName << " : " << testStageVal << std::endl;
    } else {
        throw "Engine not initialized" ;
    }
}

void DebugEngineWrapper::stopTest(bool result_) {
    if ( debugEngine ) {
        std::cout << "DEBUG ENGINE: End of Test -- Result = " << result_ << std::endl;
    } else {
        throw "Engine not initialized" ;
    }
}

IOutputEngine* DebugEngineWrapper::getOutputEngine() {
  return debugEngine; 
}

extern "C" {
 OutputEngineManager*  DebugEngineBuilder(std::string outfile , std::string configFile , bool debug ) {
   return new DebugEngineWrapper(outfile, configFile, debug);
 }
}

class Debug_engine_proxy {                                                                               
public:                                                                                                       
  Debug_engine_proxy(){                                                                                  
    VnV_registerEngine("debug",DebugEngineBuilder);                
  }                                                                                                           
};                                                                                                            
                                                                                                            
Debug_engine_proxy debug_engine_proxy; 


