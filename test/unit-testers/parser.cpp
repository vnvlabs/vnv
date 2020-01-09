#ifndef _EuclideanError_H 
#define _EuclideanError_H 

#include "VnV-Interfaces.h"
#include "VnV.h"
#include <sstream>
using namespace VnV;

class ParserUnitTests : public IUnitTester { 

   int x,y;	
public:

    ParserUnitTests(int _x, int _y ) : x(_x), y(_y) {

    }

    std::map<std::string,bool> run(IOutputEngine* /* engine */) {
        
	std::map<std::string, bool> results;
        std::string xstr = std::to_string(x);
        std::string ystr = std::to_string(y);
        std::ostringstream t1,t2,t3;
        t1 << "x(" << x << ") + y(" << y << ") == 22 (" << x+y << ")";
        t2 << "x(" << x << ") - y(" << y << ") == -2 (" << x-y << ")";
        t3 << "x(" << x << ") != y(" << y << ")";


        results[t1.str()] = ( (x + y) == 22);
        results[t2.str()] = ( (x - y) == -2 );
        results[t3.str()] = ( x != y );
        results["x == 10"] = (x==10);
        results["y == 12"] = (y==12);
        return results;
    }

    ~ParserUnitTests();
};

ParserUnitTests::~ParserUnitTests(){};

IUnitTester* parser_maker() {
    return new ParserUnitTests(10,10);
}
IUnitTester* parser_maker1() {
    return new ParserUnitTests(0,0);
}
IUnitTester* parser_maker2() {
    return new ParserUnitTests(10,12);
}
IUnitTester* parser_maker3() {
    return new ParserUnitTests(-1,-22);
}

void parser_callBack() {
    VnV_registerUnitTester("parser_10_10", parser_maker);
    VnV_registerUnitTester("parser_0_0", parser_maker1);
    VnV_registerUnitTester("parser_10_12", parser_maker2);
    VnV_registerUnitTester("parser_n1_n22", parser_maker3);
}

#endif
