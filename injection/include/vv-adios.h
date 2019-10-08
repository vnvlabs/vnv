#ifndef VV_ADIOS_HEADER
#define VV_ADIOS_HEADER

#include <string>
#include "adios2.h"

namespace VnV {

class IOutputEngine;
class OutputEngineManager;

class AdiosEngine : public IOutputEngine {
  public:
    
     adios2::IO &writer;
     adios2::Engine &engine;

     AdiosEngine(adios2::Engine &_engine, adios2::IO &_io);
  
     void Put(std::string variableName, double &value) ;   
     void Put(std::string variableName, int &value) ;   
     void Put(std::string variableName, float &value) ;   
     void Put(std::string variableName, long &value) ;   
    
     void DefineDouble(std::string name) ;
     void DefineFloat(std::string name) ;
     void DefineInt(std::string name) ;
     void DefineLong(std::string name) ;

};



class AdiosWrapper : public OutputEngineManager {
private:
    adios2::ADIOS adios;
    adios2::IO bpWriter;
    adios2::Engine engine;
    adios2::Variable<std::string> identifier;
    adios2::Variable<int> stage;
    adios2::Variable<std::string> type;
    adios2::Variable<std::string> markdown;
    adios2::Variable<int> result;
    unsigned int outputFile;

    AdiosEngine *adiosEngine;

public:

    AdiosWrapper(std::string outfile, bool debug );
    AdiosWrapper(std::string outfile, std::string configFile, bool debug );
    
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
