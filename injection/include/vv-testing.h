
#ifndef VV_TESTING_HEADER
#define VV_TESTING_HEADER


#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <dlfcn.h>
#include <set>
#include <stdarg.h>

#include "vv-runtime.h"
#include "vv-output.h"

#ifdef __cplusplus
    #define EXTERNC extern "C" 
#else
    #define EXTERNC 
#endif

// Some simplifying typedefs. 
typedef std::map<std::string, std::pair<std::string,void*> > NTV;
typedef std::map<std::string, std::string> NT;

enum TestStatus {SUCCESS,FAILURE, NOTRUN};

// Transform interface. 
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

// Interface for writing a test. 
class IVVTest {
public:

    VVTestConfig m_config;
    NT m_parameters;
    bool typeChecking = true;
    std::map<std::string, std::string > adiosVariables;

    IVVTest(VVTestConfig &config);
    virtual ~IVVTest();
    TestStatus  _runTest( IVVOutputEngine* engine, int stageVal, NTV &params);

    virtual TestStatus runTest( IVVOutputEngine *engine, int stage , NTV &params) = 0;

protected:
    template <typename T>
    T* carefull_cast(int stage, std::string parameterName, NTV &parameters );
};

typedef IVVTest *maker_ptr(VVTestConfig&);
typedef void variable_register_ptr(IVVOutputEngine*);
typedef IVVTransform *trans_ptr();

EXTERNC void VV_registerTest(std::string name, maker_ptr m , variable_register_ptr v);
EXTERNC void VV_registerTransform(std::string name, trans_ptr t);


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
