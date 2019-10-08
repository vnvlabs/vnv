
#ifndef VV_XMLPARSER_HEADER
#define VV_XMLPARSER_HEADER

#include <string>
#include <set>
#include "vv/all-include.h" // Include the XSD2CPP cpp source code 
#include "vv-parser.h"

namespace VnV {

class XmlParser : public IParser {
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

