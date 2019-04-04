
#ifndef INJECTION_POINTS_H
#define INJECTION_POINTS_H

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <set>
#include "vv_macros.h"
#include "vv/all-include.h" // Include the XSD2CPP cpp source code 
#include "adios2.h"
#include <stdarg.h>


typedef std::vector<std::pair<std::string,std::string>> plist;
typedef std::map<std::string, std::pair<std::string,void*> > NTV; 
typedef std::map<std::string, std::string> NT;

enum TestLibraryStatus { OPEN, CLOSED, ERROR }; 
enum TestStatus {SUCCESS,FAILURE, NOTRUN};

// This class defines a testing library. This manages opening 
// the library with dlopen and closing it with dlclose. The tests
// in the library must use the self registering framework to be 
// available in the main project. 


/*class FileSystemTracker {
  public:
    static std::map<std::string, std::pair<std::string, int> > fileaccesses;
    static void Open(const char * fname, const char *mode);
    static std::string Print();
};*/

class VVTestLibrary {

public:
  std::string libraryPath;
  void* dlib_entry; 
  TestLibraryStatus status = CLOSED;

  VVTestLibrary(std::string path);
  ~VVTestLibrary();

  void openLibrary();
  void closeLibrary();

};

class IVVTransform {
  public:
    IVVTransform();
    virtual void* Transform(std::pair<std::string, void*> ip, std::string tp ) = 0;    
};

class DefaultVVTransform : public IVVTransform {
    virtual void* Transform(std::pair<std::string, void*> ip, std::string tp);
};

class VVTestStageConfig {
public:
  
  static DefaultVVTransform defaultTransform; 
  
  
  int injectionPointStageId;
  int testStageId; 
  std::map<std::string, std::pair< std::string, IVVTransform* >> transforms; // maps testParameter -> ip_parameter & Transform 

  void addTransform(std::string to, std::string from, std::string trans);

  std::pair<std::string, IVVTransform*>  getTransform(std::string);
};

// Now we define the tests -- These are the tests that we run 
class VVTestConfig {
  

  public:
  std::string testName;
  std::string markdown;

  std::map<int, VVTestStageConfig> stages;

  bool runOnStage(int stage);

  void addTestStage(VVTestStageConfig config);

  VVTestStageConfig getStage(int stage);

};

class IVVTest {
  public: 
    
    VVTestConfig m_config; 
    NT m_parameters;  
    
    std::map<std::string, std::string > adiosVariables;    
    
    bool typeChecking = true;

    IVVTest(VVTestConfig &config);
        virtual ~IVVTest();
    
  
    // These are wrappers to the above functions. These wrappers are 
    // called to start and stop tests. 
    TestStatus  _runTest( int stageVal, NTV &params);
  
    virtual TestStatus runTest( int stage , NTV &params) = 0;
    
  protected:

    template <typename T> 
    T* carefull_cast(int stage, std::string parameterName, NTV &parameters ); 
    
};


typedef IVVTest *maker_ptr(VVTestConfig&);
typedef void adios_register_ptr(adios2::IO&);
typedef IVVTransform *trans_ptr();




class InjectionPointStage {
public:

  bool m_empty = false;

  std::string m_filename;
  std::string m_desc;
  int m_lineNumber;
  plist m_params;

public:

  InjectionPointStage(std::string filename, int lineNumber, std::string desc, plist &params);
  
  InjectionPointStage(std::string markdown);

  std::string getFilename() const; 
  int getLineNumber() const;
  plist getParams() const;
  std::string getDescription() const;  
  bool isEmpty();
  void setFromEmpty(std::string filename, int lineNumber, std::string desc, plist &params);
  

};

class InjectionPoint {
public:
   
    InjectionPoint(std::string scope, int stage, std::string filename, int lineNumber, std::string desc, plist &params);
  
    InjectionPoint(std::string scope, std::string markdown);

    std::string m_scope;
    std::map<int, InjectionPointStage> stages; 
    std::vector< IVVTest* > m_tests;     
    
    std::string m_markdown;
    
    std::string getScope() const;
    
    void unpack_parameters(int stageVal, NTV &ntv, va_list argp);

    void addStage(int stageIndex, std::string filename, int lineNumber, std::string desc, plist &params);

    void runTests(int ipType, va_list argp); 
   
    void addTest(VVTestConfig c);

};



class IntroStruct {
  public:
  std::string intoMarkdown;
};

class OutroStruct {
  public:
  std::string outroMarkdown;
};


typedef std::map<std::string, InjectionPoint > ip_map;

class InjectionPointBaseFactory {
public:
  static InjectionPoint getDescription(std::string const& id); 
  static void getAll(); // Prints out all the injection points. 
  
  static void writeIntroduction();
  static void writeConclusion();

  static void addTest(vv::test_p , InjectionPoint& );
  static void addInjectionPoint(std::string scope, vv::injectionPoint_p);
  static void addTestLibrary(vv::testLibrary_p);
  static void addScope(vv::scope_p, std::string level = "");
  static void parseXMLFile(vv::Document*); 
  
  static void writeXMLFile(std::string filename);
  static void populateDocument(vv::Document* doc);
  static void populateScope(vv::scope_p scope, InjectionPoint &ipd);
  static void parseXMLFile(std::string filename);

  static ip_map *getMap(); 
private:
  
  std::string exe_intro_md, exe_outro_md;

  static ip_map *map;
  static std::vector<void*> testLibraries;
  static IntroStruct intro;
  static OutroStruct outro;

};

enum VVTestStatus { TEST, IP } ;

class AdiosWrapper;

class InjectionPointRegistrar : InjectionPointBaseFactory {

public:
  InjectionPointRegistrar(std::string scope, int stageVal, std::string filename, int line, std::string desc, int count, ...); 

  

};


class VV {
public:

  static bool runTests; 

  static std::map <std::string, std::pair<maker_ptr *, adios_register_ptr * >  , std::less<std::string>> test_factory;
  static std::map <std::string, trans_ptr * , std::less<std::string>> trans_factory;

  static IVVTransform* getTransform(std::string tname);

  static AdiosWrapper *adiosWrapper; 

  static int StringSplit(const std::string &s,const char *delim, std::vector< std::string > &result );

  static void injectionPoint(int injectionIndex, std::string scope, std::string function, ...) ;
  static void injectionPoint(int injectionIndex, std::string scope, std::string function, va_list argp) ;

  static void setAdiosWrapper(std::string outfileName="vvout.bp", std::string configFile="", MPI_Comm comm=MPI_COMM_WORLD, bool debug=true);

  static bool VVInit(std::string configFile); 
  static bool VVFinalize();

  static void writeXMLFile(std::string filename);

};

extern "C" {
  void VV_injectionPoint(int stageVal, const char * id, const char * function, ...); 
}


class AdiosWrapper {

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
    std::set<std::string> registeredTests;

  AdiosWrapper(std::string outfile, MPI_Comm comm, bool debug ) :
     adios(comm,debug) 
  {
      setAdios(outfile);
  }

  AdiosWrapper(std::string outfile, std::string configFile, MPI_Comm comm, bool debug ) : 
      adios(configFile, comm, debug)
  {
      setAdios(outfile);
  }

  void writeIntroduction(IntroStruct intro) {
    if (engine) {
      engine.BeginStep();
      engine.Put(stage, 88888);
      engine.Put(markdown,intro.intoMarkdown);
      std::string s = "introduction";
      engine.Put(type, s);
      engine.EndStep();
    }
  }
  
  void writeConclusion(OutroStruct outro) {
    if (engine) {
      engine.BeginStep();
      engine.Put(stage, -88888);
      engine.Put(markdown, outro.outroMarkdown);
      std::string s = "conclusion";
      engine.Put(type, s);
      engine.EndStep();
    }
  }

  void DeclareTest(std::string testName) {
       auto it = VV::test_factory.find(testName);
       auto itt = registeredTests.find(testName);

       if ( itt == registeredTests.end() && it != VV::test_factory.end() ) {
          it->second.second(bpWriter);
          registeredTests.insert(testName);
       } 
            
  }

  void finalizeAdios() {
    if (engine) {
      engine.Close();
    }
  }

  void setAdios(std::string outfile) {
      bpWriter = adios.DeclareIO("BPWriter");
      //bpWriter.SetEngine("HDF5");
      outputFile = bpWriter.AddTransport("File",{{"Library","POSIX"},{"Name",outfile.c_str()}});
      identifier = bpWriter.DefineVariable<std::string>("identifier");
      stage = bpWriter.DefineVariable<int>("stage");
      type = bpWriter.DefineVariable<std::string>("type");
      markdown = bpWriter.DefineVariable<std::string>("markdown");  
      result = bpWriter.DefineVariable<int>("result");
      engine = bpWriter.Open(outfile, adios2::Mode::Write);
  

  }

  void endInjectionPoint(std::string id, int stageVal , std::string markdownVal ) {
    
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
  void startInjectionPoint(std::string id, int stageVal , std::string markdownVal ) {
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

  void startTest( std::string testName, int testStageVal, std::string markdownVal ) {
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

  void stopTest(bool result_) {
      if ( engine ) {
        int res = (result_) ? 1 : 0 ;
        engine.Put(result, res);
        engine.EndStep();    
      }
  }

};

//Include the template definitions so we can use them in other libraries. 
#include "injection.tpp"

#endif
