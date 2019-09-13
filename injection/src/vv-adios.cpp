
//#if WITH_ADIOS

#include "vv-adios.h"

AdiosEngine::AdiosEngine(adios2::Engine &e, adios2::IO &i) : engine(e) , writer(i) {}

void AdiosEngine::Put(std::string variableName, double &value) {
    engine.Put(variableName,value);
}   
void AdiosEngine::Put(std::string variableName, int &value) {
    engine.Put(variableName,value);

}   
void AdiosEngine::Put(std::string variableName, float &value) {
    engine.Put(variableName,value);
}   
void AdiosEngine::Put(std::string variableName, long &value) {
    engine.Put(variableName,value);
}   

void AdiosEngine::DefineDouble(std::string name) {
    writer.DefineVariable<double>(name);
}

void AdiosEngine::DefineFloat(std::string name) {
    writer.DefineVariable<float>(name);
}

void AdiosEngine::DefineInt(std::string name) { 
    writer.DefineVariable<int>(name);
}


void AdiosEngine::DefineLong(std::string name) {
    writer.DefineVariable<long>(name);
}



AdiosWrapper::AdiosWrapper(std::string outfile, bool debug ) :
    adios(MPI_COMM_WORLD,debug) {
    set(outfile);
}

AdiosWrapper::AdiosWrapper(std::string outfile, std::string configFile,  bool debug ) :
    adios(configFile, MPI_COMM_WORLD, debug)
{
    set(outfile);
}

void AdiosWrapper::writeIntroduction(IntroStruct intro) {
    if (engine) {
        engine.BeginStep();
        engine.Put(stage, 88888);
        engine.Put(markdown,intro.intoMarkdown);
        std::string s = "introduction";
        engine.Put(type, s);
        engine.EndStep();
    }
}

void AdiosWrapper::writeConclusion(OutroStruct outro) {
    if (engine) {
        engine.BeginStep();
        engine.Put(stage, -88888);
        engine.Put(markdown, outro.outroMarkdown);
        std::string s = "conclusion";
        engine.Put(type, s);
        engine.EndStep();
    }
}


void AdiosWrapper::finalize() {
    if (engine) {
        engine.Close();
    }
}

void AdiosWrapper::set(std::string outfile) {
    bpWriter = adios.DeclareIO("BPWriter");
    //bpWriter.SetEngine("HDF5");
    outputFile = bpWriter.AddTransport("File", {{"Library","POSIX"},{"Name",outfile.c_str()}});
    identifier = bpWriter.DefineVariable<std::string>("identifier");
    stage = bpWriter.DefineVariable<int>("stage");
    type = bpWriter.DefineVariable<std::string>("type");
    markdown = bpWriter.DefineVariable<std::string>("markdown");
    result = bpWriter.DefineVariable<int>("result");
    engine = bpWriter.Open(outfile, adios2::Mode::Write);

    adiosEngine = new AdiosEngine(engine,bpWriter);

}

void AdiosWrapper::endInjectionPoint(std::string id, int stageVal , std::string markdownVal ) {

    if ( engine ) {
        engine.BeginStep();
        engine.Put(identifier, id);
        engine.Put(stage, stageVal );
        engine.Put(markdown, markdownVal);
        std::string s = "EndIP";
        engine.Put(type, s);
        engine.EndStep();
    } else {
        throw "Engine not initialized" ;
    }
}

void AdiosWrapper::startInjectionPoint(std::string id, int stageVal , std::string markdownVal ) {
    if ( engine ) {
        engine.BeginStep();
        engine.Put(identifier, id);
        engine.Put(stage, stageVal );
        engine.Put(markdown, markdownVal);
        std::string s = "StartIP";
        engine.Put(type, s);
        engine.EndStep();
    } else {
        throw "Engine not initialized" ;
    }
}

void AdiosWrapper::startTest( std::string testName, int testStageVal, std::string markdownVal ) {
    if (engine) {
        engine.BeginStep();
        engine.Put(identifier, testName);
        engine.Put(stage, testStageVal);
        engine.Put(markdown, markdownVal);
        std::string test = "StartTest";
        engine.Put(type,test);
    } else {
        throw "Engine not setup correctly";
    }
}

void AdiosWrapper::stopTest(bool result_) {
    if ( engine ) {
        int res = (result_) ? 1 : 0 ;
        engine.Put(result, res);
        engine.EndStep();
    }
}

IVVOutputEngine* AdiosWrapper::getOutputEngine() {
  return adiosEngine; 
}

extern "C" {
 VVOutputEngineManager*  AdiosEngineBuilder(std::string outfile , std::string configFile , bool debug ) {
   return new AdiosWrapper(outfile, configFile, debug);
 }
}

class Adios_engine_proxy {                                                                               
public:                                                                                                       
  Adios_engine_proxy(){                                                                                  
    VV_registerEngine("adios",AdiosEngineBuilder);                
  }                                                                                                           
};                                                                                                            
                                                                                                            
Adios_engine_proxy adios_engine_proxy; 


//#endif
