
#ifndef VV_TESTING_HEADER
#define VV_TESTING_HEADER


#include <string>
#include <map>
#include <vector>
#include <set>

#include "vv-output.h"
#include <iostream>
namespace VnV {

// Some simplifying typedefs. 
typedef std::map<std::string, std::pair<std::string,void*> > NTV;
typedef std::map<std::string, std::string> NT;

enum TestStatus {SUCCESS,FAILURE, NOTRUN};

// Transform interface. 
class ITransform {
public:
    ITransform();   
    virtual void* Transform(std::pair<std::string, void*> ip, std::string tp ) = 0;
};

class DefaultTransform : public ITransform {
    virtual void* Transform(std::pair<std::string, void*> ip, std::string tp);
};


class TestStageConfig {
  private:
    int injectionPointStageId;
    int testStageId;
    
    std::map<std::string, std::pair< std::string, std::string >> transforms; // maps testParameter -> ip_parameter & Transform
  
  public:

    static DefaultTransform defaultTransform;
    void addTransform(std::string to, std::string from, std::string trans);
    
    int getInjectionPointStageId();
    int getTestStageId();
    void setInjectionPointStageId(int id);
    void setTestStageId(int id);

    std::pair<std::string, ITransform*>  getTransform(std::string);
};

// Now we define the tests -- These are the tests that we run
class TestConfig {
  private:
    std::string testName;
    std::map<std::string, std::string> config_params;
    std::map<int, TestStageConfig> stages;
    std::set<std::string> scopes;
  
  public:
    bool runOnStage(int stage);
    void addTestStage(TestStageConfig config);
    TestStageConfig getStage(int stage);

    void setName(std::string name);
    std::string getName();

    void addParameter(const std::pair<std::string, std::string> &pair) ;


};

// Interface for writing a test. 
class ITest {
public:


    ITest(TestConfig &config);
    virtual ~ITest();
    TestStatus  _runTest( IOutputEngine* engine, int stageVal, NTV &params);

    virtual TestStatus runTest( IOutputEngine *engine, int stage , NTV &params) = 0;

protected:
    TestConfig m_config;
    NT m_parameters;
    bool typeChecking = true;
    
    template <typename T>
    T* carefull_cast(int stage, std::string parameterName, NTV &parameters );

    
};


typedef ITest *maker_ptr(TestConfig&);
typedef void variable_register_ptr(IOutputEngine*);
typedef ITransform *trans_ptr();



class TestStore {

private:
    std::vector<void*> testLibraries;
    std::set<std::string> registeredTests;
    std::map <std::string, std::pair<maker_ptr *, variable_register_ptr * >  , std::less<std::string>> test_factory;
    std::map <std::string, trans_ptr * , std::less<std::string>> trans_factory;
    
    TestStore();

public: 
      
    void addTestLibrary(std::string libraryPath);
    void addTest(std::string name, maker_ptr m, variable_register_ptr v);
    void addTransform(std::string name, trans_ptr p);


    ITransform* getTransform(std::string tname);
    
    ITest* getTest(TestConfig &config);

    static TestStore & getTestStore(); 
};



template <typename T>
T* ITest::carefull_cast(int stage, std::string parameterName, NTV &parameters ) {

    // First, make sure "parameterName" is a test parameter and, if it is, get its type
    
    auto test_parameter = m_parameters.find(parameterName);
    if ( test_parameter == m_parameters.end() ) {
        throw " This is not a test parameter ";
    }

    //Next, get the transform. If one exists, it is returned, else return "parameterName,DefaultTransform">
    std::pair<std::string, ITransform*>  trans = m_config.getStage(stage).getTransform(parameterName);

    auto ip_parameter = parameters.find(trans.first);
    
    
    if ( ip_parameter == parameters.end() ) {
        throw "A injection point parameter with the transform name does not exist" ;
    }

    // Transform the injection point parameter into the test parameter
    void * tptr = trans.second->Transform(ip_parameter->second, test_parameter->second);

    // Finally, cast it to the correct type;
    return (T*) tptr;
}

}

#ifdef __cplusplus
    #define EXTERNC extern "C" 
#else
    #define EXTERNC 
#endif

EXTERNC void VnV_registerTest(std::string name, VnV::maker_ptr m , VnV::variable_register_ptr v);
EXTERNC void VnV_registerTransform(std::string name, VnV::trans_ptr t);

#undef EXTERNC

#endif
