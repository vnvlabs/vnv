
#include "vv-testing.h"
#include "injection.h"


DefaultVVTransform VVTestStageConfig::defaultTransform;

bool VVTestConfig::runOnStage(int index) {

    return ( stages.find(index) != stages.end());
}

void VVTestConfig::addTestStage(VVTestStageConfig config) {

    stages.insert(std::make_pair(config.injectionPointStageId, config));
}

VVTestStageConfig VVTestConfig::getStage(int stage) {

    auto it = stages.find(stage);
    if ( it != stages.end() )
        return it->second;
    throw "Stage not found " ;
}

void VVTestStageConfig::addTransform(std::string testParameter, std::string ipParameter, std::string trans) {
    IVVTransform *transform = InjectionPointBaseFactory::getTransform(trans);
    transforms.insert( std::make_pair(testParameter, std::make_pair(ipParameter, transform)));
}

std::pair<std::string, IVVTransform*> VVTestStageConfig::getTransform(std::string s) {

    auto it = transforms.find(s);
    if ( it != transforms.end())
        return it->second;

    // Could not find a transform for that parameter, so return the default transform with same name;
    return std::make_pair(s, &defaultTransform);
}


IVVTransform::IVVTransform()  {}

void * DefaultVVTransform::Transform(std::pair<std::string, void*> ip, std::string testParameterType) {

    if ( ip.first.compare(testParameterType) == 0 )
        return ip.second;
    throw "Default Transform does not apply";
}


IVVTest::IVVTest(VVTestConfig &config) : m_config(config) {

}


// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus IVVTest::_runTest(IVVOutputEngine *engine, int stageVal, NTV &params) {
    if ( m_config.runOnStage(stageVal) )   {
        int testVal = m_config.getStage(stageVal).testStageId;
        InjectionPointBaseFactory::manager->startTest(m_config.testName, testVal, m_config.markdown);
        TestStatus s = runTest(engine, stageVal, params);
        InjectionPointBaseFactory::manager->stopTest(( s==SUCCESS) ? true: false);
        return s;
    }
    return NOTRUN;
}

IVVTest::~IVVTest() {

}

void VV_registerTest(std::string name, maker_ptr m , variable_register_ptr v) {
  InjectionPointBaseFactory::test_factory[name] = std::make_pair(m,v);
}
void VV_registerTransform(std::string name, trans_ptr t) {
  InjectionPointBaseFactory::trans_factory[name] = t;
  
}

