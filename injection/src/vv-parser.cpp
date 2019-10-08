
#include <string>

#include "vv-parser.h"
#include "vv-testing.h"
#include "vv-injection.h"
#include "vv-utils.h"
#include "vv-output.h"

using namespace vv;
using namespace VnV;


void Parser::addTest(test_p t, std::vector<TestConfig> &testConfigs, std::set<std::string> &runScopes) {
    std::string testName = t->get_attr_name_string();
   
    if ( runScopes.size() != 0 ) {
        
       bool add = false;
       int i = 0;
       for ( auto s : t->elements_runScope() ) {
          if ( runScopes.find(t->get_runScope_string(i++)) != runScopes.end() ) {
            add = true;
            break;
          }
        }
        if (!add) return;  
    }

    TestConfig c;
    c.setName(t->get_attr_name_string());
    
    for ( auto p : t->elements_testStage() ) {
        TestStageConfig config;
        config.setTestStageId( p->get_attr_testId() );
        config.setInjectionPointStageId( p->get_attr_ipId());
        for ( auto param : p->elements_parameter() ) {
            config.addTransform(param->get_attr_to_string(), param->get_attr_from_string(), param->get_attr_trans_string());
        }
        for (auto conf : t->elements_config() ) {
            c.addParameter( std::make_pair( conf->get_attr_name_string(),conf->get_attr_value_string()));
        }
        c.addTestStage(config);
    }
    testConfigs.push_back(c);
}

void Parser::addInjectionPoint(injectionPoint_p ip, std::set<std::string> &runScopes, std::map<std::string,std::vector<TestConfig>> &ips) {

    std::string name = ip->get_attr_name_string();

    auto aip = ips.find(name);
    if ( aip != ips.end() ) {      
        for (auto test : ip->elements_test() ) {
           addTest(test, aip->second,runScopes);
        }    
    } else {
        std::vector<TestConfig> testConfigs; 
        for (auto test : ip->elements_test() ) {
           addTest(test, testConfigs, runScopes);
        } 

        if ( testConfigs.size() > 0) {
          ips.insert(std::make_pair(name,testConfigs));
        }
    }
}

void Parser::addTestLibrary(testLibrary_p lib, std::set<std::string> &libs) {
    int i = 0;   
    for ( auto it : lib->elements_path() ) {
        libs.insert(lib->get_path_string(i++));
    }  
}

EngineInfo Parser::getEngineInfo(outputEngine_p e) {
    EngineInfo info;
    info.engineType = e->get_type_string();
    info.outFileName = e->get_outputFile_string();
    info.engineConfig = e->get_configFile_string();
    info.debug = e->get_debug();
    return info;
}

RunInfo Parser::parseXMLFile(vv::Document *doc ) {

    RunInfo info;

    exe_p exe = doc->element_exe();
    
    std::set<std::string> runScopes;
    int i = 0;
    

    runScopes_ptr rs_ptr = exe->element_runScopes();
    
    info.runTests = rs_ptr->get_attr_run();
    
    
    if ( info.runTests == false ) { 

      return info;
    }
    
    outputEngine_p oe_ptr = exe->element_outputEngine();
    info.engineInfo = getEngineInfo(oe_ptr);
    
    for (auto rS : rs_ptr->elements_scope()) {
        runScopes.insert(rs_ptr->get_scope_string(i++));
    }

    testLibraries_ptr tl_ptr = exe->element_testLibraries();
    for ( auto lib : tl_ptr->elements_testLibrary() ) {
        addTestLibrary(lib, info.testLibraries);
    }

    injectionPoints_ptr ip_ptr = exe->element_injectionPoints();
    for ( auto ip : ip_ptr->elements_injectionPoint() ) {
        addInjectionPoint(ip, runScopes, info.injectionPoints);
    }
    
    return info;
}

RunInfo Parser::parse(std::string filename) {

    try {
        vv::Document * doc = new vv::Document(false);
        doc->prettyPrint(true);

        ifstream ifs(filename.c_str());
        ifs >> *doc;
        return parseXMLFile(doc);

    }
    catch(XPlus::Exception& ex) {
        
        RunInfo info;
        info.runTests = false;
        info.error = true;
        info.errorMessage = ex.msg();
        return info;
    }


}




