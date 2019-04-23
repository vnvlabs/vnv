
#ifndef INJECTION_POINTS_H
#define INJECTION_POINTS_H

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <set>
#include "vv/all-include.h" // Include the XSD2CPP cpp source code 
#include "adios2.h"
#include <stdarg.h>

#include "vv-runtime.h"

typedef std::vector<std::pair<std::string,std::string>> plist;
typedef std::map<std::string, std::pair<std::string,void*> > NTV;
typedef std::map<std::string, std::string> NT;

enum TestLibraryStatus { OPEN, CLOSED, ERROR };
enum TestStatus {SUCCESS,FAILURE, NOTRUN};


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
    std::map<std::string, std::string> config_params;

    std::map<int, VVTestStageConfig> stages;
    bool runOnStage(int stage);
    void addTestStage(VVTestStageConfig config);
    VVTestStageConfig getStage(int stage);

};

class IVVTest {
public:

    VVTestConfig m_config;
    NT m_parameters;
    bool typeChecking = true;
    std::map<std::string, std::string > adiosVariables;

    IVVTest(VVTestConfig &config);
    virtual ~IVVTest();
    TestStatus  _runTest( adios2::Engine &engine, int stageVal, NTV &params);

    virtual TestStatus runTest( adios2::Engine &engine, int stage , NTV &params) = 0;

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


    std::string m_scope;
    std::map<int, InjectionPointStage> stages;
    std::vector< IVVTest* > m_tests;
    std::string m_markdown;

    InjectionPoint(std::string scope, int stage, std::string filename, int lineNumber, std::string desc, plist &params);
    InjectionPoint(std::string scope, std::string markdown);

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

    static ip_map *getMap();
    std::string exe_intro_md, exe_outro_md;
    static ip_map *map;
    static std::vector<void*> testLibraries;
    static IntroStruct intro;
    static OutroStruct outro;

    static void getAll(); // Prints out all the injection points.
    static InjectionPoint getDescription(std::string const& id);
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
};

class InjectionPointRegistrar : InjectionPointBaseFactory {
public:
    InjectionPointRegistrar(std::string scope, int stageVal, std::string filename, int line, std::string desc, int count, ...);
    InjectionPointRegistrar(std::string scope, int stageVal, std::string filename, int line, std::string desc, int count, va_list argp);
};

class AdiosWrapper;

class VV {
public:

    static bool runTests;
    static AdiosWrapper *adiosWrapper;
    static std::map <std::string, std::pair<maker_ptr *, adios_register_ptr * >  , std::less<std::string>> test_factory;
    static std::map <std::string, trans_ptr * , std::less<std::string>> trans_factory;

    static IVVTransform* getTransform(std::string tname);
    static int StringSplit(const std::string &s,const char *delim, std::vector< std::string > &result );
    static void injectionPoint(int injectionIndex, std::string scope, std::string function, va_list argp) ;
    static void setAdiosWrapper(std::string outfileName="vvout.bp", std::string configFile="", MPI_Comm comm=MPI_COMM_WORLD, bool debug=true);
    
    
    static void injectionPoint(int injectionIndex, std::string scope, std::string function, ...) ;
    static bool VVInit(std::string configFile);
    static bool VVFinalize();
    static void writeXMLFile(std::string filename);

};

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

    AdiosWrapper(std::string outfile, MPI_Comm comm, bool debug );
    AdiosWrapper(std::string outfile, std::string configFile, MPI_Comm comm, bool debug );
    void writeIntroduction(IntroStruct intro);
    void writeConclusion(OutroStruct outro);
    void DeclareTest(std::string testName) ;
    void finalizeAdios();
    void setAdios(std::string outfile);
    void endInjectionPoint(std::string id, int stageVal , std::string markdownVal ) ;
    void startInjectionPoint(std::string id, int stageVal , std::string markdownVal );
    void startTest( std::string testName, int testStageVal, std::string markdownVal );
    void stopTest(bool result_) ;
};

template <typename T>
T* IVVTest::carefull_cast(int stage, std::string parameterName, NTV &parameters ) {

    // First, make sure "parameterName" is a test parameter and, if it is, get its type
    auto test_parameter = m_parameters.find(parameterName);
    if ( test_parameter == m_parameters.end() ) {
        throw " This is not a test parameter ";
    }

    //Next, get the transform. If one exists, it is returned, else return "parameterName,DefaultTransform">
    std::pair<std::string, IVVTransform*>  trans = m_config.getStage(stage).getTransform(parameterName);

    //Next, get the parameter from the injection point parameters that the transform says we want.
    auto ip_parameter = parameters.find(trans.first);
    if ( ip_parameter == parameters.end() ) {
        throw "A injection point parameter with the transform name does not exist" ;
    }

    // Transform the injection point parameter into the test parameter
    // Transforms should check the types
    void * tptr = trans.second->Transform(ip_parameter->second, test_parameter->second);

    // Finally, cast it to the correct type;
    return (T*) tptr;
}

#endif
