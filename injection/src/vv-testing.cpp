#include <iostream>
#include <dlfcn.h>
#include "vv-testing.h"
#include "vv-output.h"

using namespace VnV;

DefaultTransform TestStageConfig::defaultTransform;

bool TestConfig::runOnStage(int index) {

    return ( stages.find(index) != stages.end());
}

void TestConfig::addTestStage(TestStageConfig config) {

    stages.insert(std::make_pair(config.getInjectionPointStageId(), config));
}

void TestConfig::setName(std::string name){
  testName = name;
}

void TestConfig::addParameter(const std::pair<std::string, std::string> &pair) {
   config_params.insert(pair);
}

std::string TestConfig::getName() {
  return testName;
}

TestStageConfig TestConfig::getStage(int stage) {

    auto it = stages.find(stage);
    if ( it != stages.end() )
        return it->second;
    throw "Stage not found " ;
}

void TestStageConfig::addTransform(std::string testParameter, std::string ipParameter, std::string trans) {
    transforms.insert( std::make_pair(testParameter, std::make_pair(ipParameter, trans)));
}


int TestStageConfig::getInjectionPointStageId() {
  return injectionPointStageId;
}

int TestStageConfig::getTestStageId() {
  return testStageId;
}

void TestStageConfig::setTestStageId(int id) {
  testStageId = id;
}

void TestStageConfig::setInjectionPointStageId(int id) {
  injectionPointStageId = id;
}

std::pair<std::string, ITransform*> TestStageConfig::getTransform(std::string s) {

    auto it = transforms.find(s);
    if ( it != transforms.end()) {
        ITransform *transform = TestStore::getTestStore().getTransform(it->second.second);
        return std::make_pair(it->second.first, transform);
    }

    // Could not find a transform for that parameter, so return the default transform with same name;
    return std::make_pair(s, &defaultTransform);
}


ITransform::ITransform()  {}

void * DefaultTransform::Transform(std::pair<std::string, void*> ip, std::string testParameterType) {

    if ( ip.first.compare(testParameterType) == 0 )
        return ip.second;
    throw "Default Transform does not apply";
}


ITest::ITest(TestConfig &config) : m_config(config) {

}


// Index is the injection point index. That is, the injection
// point that this test is being run inside.
TestStatus ITest::_runTest(IOutputEngine *engine, int stageVal, NTV &params) {
     
   if ( m_config.runOnStage(stageVal) )   {
        int testVal = m_config.getStage(stageVal).getTestStageId();
        EngineStore::getEngineStore().getEngineManager()->startTest(m_config.getName(), testVal);
        TestStatus s = runTest(engine, stageVal, params);
        EngineStore::getEngineStore().getEngineManager()->stopTest(( s==SUCCESS) ? true: false);
        return s;
    }
    return NOTRUN;
}

ITest::~ITest() {

}



TestStore::TestStore(){}

TestStore& TestStore::getTestStore() {
  static TestStore *store = new TestStore();
  return *store;  
}

void TestStore::addTestLibrary(std::string libraryPath) {
    try {
        void * dllib = dlopen(libraryPath.c_str(), RTLD_NOW);
        if ( dllib == NULL )
            std::cerr << dlerror();
        else {
           testLibraries.push_back( dllib );
        }
    } catch(...) {
        std::cout << "Library not found: " << libraryPath << "\n";
    }
}

void TestStore::addTest(std::string name, maker_ptr m , variable_register_ptr v) {
  test_factory[name] = std::make_pair(m,v);
}

ITest* TestStore::getTest(TestConfig &config) {
  
  std::string name = config.getName();
  
  auto it = test_factory.find(name);
  if ( it != test_factory.end() ) {
      auto itt = registeredTests.find(name); 
      if ( itt == registeredTests.end() ) {
        it->second.second( EngineStore::getEngineStore().getEngineManager()->getOutputEngine());
        registeredTests.insert(name);
      }
      return it->second.first(config);
  }
  return NULL;
}


ITransform* TestStore::getTransform(std::string name) {

    auto it = TestStore::getTestStore().trans_factory.find(name);

    if ( it != TestStore::getTestStore().trans_factory.end() ) {
        return it->second();
    }
    return new DefaultTransform();
}

void TestStore::addTransform(std::string name, trans_ptr t) {
    trans_factory.insert(std::make_pair(name,t));
}


void VnV_registerTest(std::string name, maker_ptr m , variable_register_ptr v) {
  TestStore::getTestStore().addTest(name, m,v);
}

void VnV_registerTransform(std::string name, trans_ptr t) {
  TestStore::getTestStore().addTransform(name,t);
  
}
