
#ifndef INJECTION_POINTS_H
#define INJECTION_POINTS_H

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <set>
#include <stdarg.h>

#include "vv-runtime.h"
#include "vv-output.h"
#include "vv-testing.h"
#include "vv/all-include.h" // Include the XSD2CPP cpp source code 

typedef std::vector<std::pair<std::string,std::string>> plist;


enum TestLibraryStatus { OPEN, CLOSED, ERROR };

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



typedef std::map<std::string, InjectionPoint > ip_map;

class InjectionPointBaseFactory {
public:

    std::string exe_intro_md, exe_outro_md;
    
    static ip_map *getMap();
    static ip_map *map;
    static std::vector<void*> testLibraries;
    static IntroStruct intro;
    static OutroStruct outro;
    static std::set<std::string> registeredTests;

    static void DeclareTest(std::string testName); 
    
    static void getAll(); // Prints out all the injection points.
    static InjectionPoint getDescription(std::string const& id);
    static void writeIntroduction();
    static void writeConclusion();
    static void addTest(vv::test_p , InjectionPoint& );
    static void addInjectionPoint(std::string scope, vv::injectionPoint_p);
    static void addTestLibrary(vv::testLibrary_p);
    static void addScope(vv::scope_p, std::string level = "");
    static void parseXMLFile(vv::Document*);
    static void populateDocument(vv::Document* doc);
    static void populateScope(vv::scope_p scope, InjectionPoint &ipd);
    static void parseXMLFile(std::string filename);
    static bool runTests;
    static VVOutputEngineManager *manager;
    static std::map <std::string, std::pair<maker_ptr *, variable_register_ptr * >  , std::less<std::string>> test_factory;
    static std::map <std::string, trans_ptr * , std::less<std::string>> trans_factory;
    static std::map<std::string, engine_register_ptr*> registeredEngines;
    static IVVTransform* getTransform(std::string tname);
    static int StringSplit(const std::string &s,const char *delim, std::vector< std::string > &result );
    static void injectionPoint(int injectionIndex, std::string scope, std::string function, va_list argp) ; 
    static void setEngineManager(std::string type, std::string outfileName="vvout.bp", std::string configFile="", bool debug=true);
    static void injectionPoint(int injectionIndex, std::string scope, std::string function, ...) ;
    static bool VVInit(std::string configFile);
    static bool VVFinalize();
    static void writeXMLFile(std::string filename);
    static void printAllEngines();
};

class InjectionPointRegistrar : InjectionPointBaseFactory {
public:
    InjectionPointRegistrar(std::string scope, int stageVal, std::string filename, int line, std::string desc, int count, ...);
    InjectionPointRegistrar(std::string scope, int stageVal, std::string filename, int line, std::string desc, int count, va_list argp);
};




#endif
