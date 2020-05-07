
#ifndef sampleTest_H 
#define sampleTest_H 

#include "VnV.h"
#include "interfaces/IUnitTester.h"

#include <sstream>
using namespace VnV;

class sampleTest : public IUnitTester { 

public:

    sampleTest( ){

    }

    std::map<std::string,bool> run(IOutputEngine* /* engine */) {
        
        std::map<std::string, bool> results;
        return results;
    }

    ~sampleTest();
};

sampleTest::~sampleTest(){};

IUnitTester* sampleTest_maker() {
    return new sampleTest();
}

void sampleTest_callBack() {
    VnV::registerUnitTester("sampleTest", sampleTest_maker);
}

#endif
