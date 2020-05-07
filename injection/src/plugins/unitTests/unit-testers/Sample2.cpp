
#ifndef Sample2_H 
#define Sample2_H 

#include "VnV.h"
#include "interfaces/IUnitTester.h"

#include <sstream>
using namespace VnV;

class Sample2 : public IUnitTester { 

public:

    Sample2( ){

    }

    std::map<std::string,bool> run(IOutputEngine* /* engine */) {
        
        std::map<std::string, bool> results;
        return results;
    }

    ~Sample2();
};

Sample2::~Sample2(){};

IUnitTester* Sample2_maker() {
    return new Sample2();
}

void Sample2_callBack() {
    VnV::registerUnitTester("Sample2", Sample2_maker);
}

#endif
