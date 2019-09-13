#ifndef VV_ADIOS_HEADER
#define VV_ADIOS_HEADER

#include "vv-output.h"
#include "adios2.h"

class AdiosEngine : public IVVOutputEngine {
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



class AdiosWrapper : public VVOutputEngineManager {

public:

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

    AdiosWrapper(std::string outfile, bool debug );
    AdiosWrapper(std::string outfile, std::string configFile, bool debug );
    void writeIntroduction(IntroStruct intro);
    void writeConclusion(OutroStruct outro);
    void DeclareTest(std::string testName) ;
    void finalize();
    void set(std::string outfile);
    void endInjectionPoint(std::string id, int stageVal , std::string markdownVal ) ;
    void startInjectionPoint(std::string id, int stageVal , std::string markdownVal );
    void startTest( std::string testName, int testStageVal, std::string markdownVal );
    void stopTest(bool result_) ;
    
    IVVOutputEngine* getOutputEngine();


};


#endif
