#ifndef VV_OUTPUT_HEADER
#define VV_OUTPUT_HEADER

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <set>
#include <stdarg.h>

#ifdef __cplusplus
    #define EXTERNC extern "C" 
#else
    #define EXTERNC 
#endif

class IntroStruct {
public:
    std::string intoMarkdown;
};

class OutroStruct {
public:
    std::string outroMarkdown;
};

class IVVOutputEngine {
  public:
  virtual void Put(std::string variableName, double &value) = 0;   
  virtual void Put(std::string variableName, int &value) = 0;   
  virtual void Put(std::string variableName, float &value) = 0;   
  virtual void Put(std::string variableName, long &value) = 0;   

  virtual void DefineDouble(std::string name) = 0;
  virtual void DefineFloat(std::string name) = 0;
  virtual void DefineInt(std::string name) = 0;
  virtual void DefineLong(std::string name) = 0;

  virtual ~IVVOutputEngine();

};


class VVOutputEngineManager {

public:

    virtual void set(std::string outfile)=0;
    virtual void writeIntroduction(IntroStruct intro)=0;
    virtual void writeConclusion(OutroStruct outro)=0;
    virtual void endInjectionPoint(std::string id, int stageVal , std::string markdownVal ) = 0;
    virtual void startInjectionPoint(std::string id, int stageVal , std::string markdownVal ) = 0;
    virtual void startTest( std::string testName, int testStageVal, std::string markdownVal ) = 0;
    virtual void stopTest(bool result_) = 0;
    virtual void finalize() = 0;
    virtual IVVOutputEngine* getOutputEngine() = 0;

    virtual ~VVOutputEngineManager(){};
};

typedef VVOutputEngineManager *engine_register_ptr(std::string , std::string, bool );


EXTERNC void VV_registerEngine(std::string name, engine_register_ptr r); 

#endif

