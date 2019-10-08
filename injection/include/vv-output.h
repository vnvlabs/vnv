#ifndef VV_OUTPUT_HEADER
#define VV_OUTPUT_HEADER

#include <string>
#include <map>


namespace VnV {

class IOutputEngine {

public:
  virtual void Put(std::string variableName, double &value) = 0;   
  virtual void Put(std::string variableName, int &value) = 0;   
  virtual void Put(std::string variableName, float &value) = 0;   
  virtual void Put(std::string variableName, long &value) = 0;   

  virtual void DefineDouble(std::string name) = 0;
  virtual void DefineFloat(std::string name) = 0;
  virtual void DefineInt(std::string name) = 0;
  virtual void DefineLong(std::string name) = 0;

  virtual ~IOutputEngine();

};


class OutputEngineManager {

public:

    virtual void set(std::string outfile)=0;
    virtual void endInjectionPoint(std::string id, int stageVal  ) = 0;
    virtual void startInjectionPoint(std::string id, int stageVal  ) = 0;
    virtual void startTest( std::string testName, int testStageVal ) = 0;
    virtual void stopTest(bool result_) = 0;
    virtual void finalize() = 0;
    virtual IOutputEngine* getOutputEngine() = 0;

    virtual ~OutputEngineManager(){};
};

typedef OutputEngineManager *engine_register_ptr(std::string , std::string, bool );

class EngineStore {
private:
  std::map<std::string, engine_register_ptr*> registeredEngines;
  OutputEngineManager *manager;
 
  EngineStore();    
  
public:
  
  void registerEngine(std::string name, engine_register_ptr* engine_ptr);
  
  OutputEngineManager* getEngineManager();
  
  void setEngineManager(std::string key, std::string outfileName="vvout.bp", std::string configFile="", bool debug=true);

  static EngineStore & getEngineStore();

};

}

#ifdef __cplusplus
    #define EXTERNC extern "C" 
#else
    #define EXTERNC 
#endif

EXTERNC void VnV_registerEngine(std::string name, VnV::engine_register_ptr r); 

#undef EXTERNC

#endif

