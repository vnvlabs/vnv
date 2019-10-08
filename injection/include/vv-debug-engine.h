
#ifndef VV_DEBUGENGINE_HEADER
#define VV_DEBUGENGINE_HEADER

#include <string>

namespace VnV {

class IOutputEngine;
class OutputEngineManager;

class DebugEngine : public IOutputEngine {
  public:
    

     DebugEngine();
  
     void Put(std::string variableName, double &value) ;   
     void Put(std::string variableName, int &value) ;   
     void Put(std::string variableName, float &value) ;   
     void Put(std::string variableName, long &value) ;   
    
     void DefineDouble(std::string name) ;
     void DefineFloat(std::string name) ;
     void DefineInt(std::string name) ;
     void DefineLong(std::string name) ;

};



class DebugEngineWrapper : public OutputEngineManager {
private:

    DebugEngine *debugEngine;

public:

    DebugEngineWrapper(std::string outfile, bool debug );
    DebugEngineWrapper(std::string outfile, std::string configFile, bool debug );
    
    void finalize();
    void set(std::string outfile);
    void endInjectionPoint(std::string id, int stageVal   ) ;
    void startInjectionPoint(std::string id, int stageVal  );
    void startTest( std::string testName, int testStageVal );
    void stopTest(bool result_) ;
    
    IOutputEngine* getOutputEngine();


};

}

#endif
