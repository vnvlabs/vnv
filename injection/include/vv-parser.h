#ifndef VV_PARSER_HEADER
#define VV_PARSER_HEADER

#include <string>
#include <set>
#include "vv/all-include.h" // Include the XSD2CPP cpp source code 
#include "vv-testing.h"
namespace VnV {

struct EngineInfo {
    std::string engineType;
    std::string engineConfig;
    std::string outFileName;
    bool debug; 
};

struct RunInfo { 

  
  bool runTests;
  std::set<std::string> testLibraries; 
  std::map<std::string, std::vector<TestConfig>> injectionPoints;

  EngineInfo engineInfo;

  bool error;
  std::string errorMessage;
};

class IParser {
public:
  virtual RunInfo parse(std::string filename) = 0;
};

class Parser : public IParser {
private:
    EngineInfo getEngineInfo(vv::outputEngine_p);
    
    void addTest(vv::test_p , std::vector<TestConfig> &testConfig, std::set<std::string> &runScopes );
    void addInjectionPoint(vv::injectionPoint_p, std::set<std::string> &runScopes, std::map<std::string,std::vector<TestConfig>> &ips);
    void addTestLibrary(vv::testLibrary_p,std::set<std::string> &libs);
    RunInfo parseXMLFile(vv::Document*);
public:   
    RunInfo parse(std::string filename);
};



}
#endif

