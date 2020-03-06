
#ifndef BFSTester_H 
#define BFSTester_H 

#include "VnV.h"
#include "interfaces/IUnitTester.h"
#include "base/Utilities.h"

#include <sstream>
using namespace VnV;

class BFSTester : public IUnitTester { 

public:

    BFSTester( ){

    }

    std::map<std::string,bool> run(IOutputEngine* /* engine */) {
        
        std::map<std::string, bool> results;

        std::map<std::string, std::map<std::string,std::string>> graph;

        // Test: Pass in a from that is not in the map --> should throw.
        try {
            VnV::bfs(graph,"from","to");
            results["FromNotInMap"] = false;
        } catch (...) {
            results["FromNotInMap"] = true;
        }
        // Test: Pass in a from == to -> should return empty vector:
        results["Empty Set When From == To "] = (0 == VnV::bfs(graph,"from","from").size());

        // Test Pass in a from that is in the map, but a to that is not.
        std::map<std::string,std::string> ss = {{"a" , "1"},{"b","2"}};
        std::map<std::string,std::string> ss1 = {{"c" , "3"},{"d","4"}};
        std::map<std::string,std::string> ss2 = {{"e" , "1"},{"f","2"}};
        std::map<std::string,std::string> ss3 = {{"f" , "1"},{"h","2"}};
        graph["from"] = ss;
        graph["a"] = ss1;
        graph["b"] = ss2;
        graph["e"] = ss3;
        // To not in graph, so should fail.
        try { VnV::bfs(graph,"from","to"); results["To not in set"] = false; } catch (...) {results["To not in set"] = true;}

        // Go from "from" to "a", a one level jump.
        try { auto x = VnV::bfs(graph,"from","a"); std::cout << x[0].first << " " << x[0].second << std::endl; results["One Level"] = (x.size()==1 && x[0].first== "a" && x[0].second == "1"); } catch (...) {results["One Level"]=false;}

        // Go from "from" to "f" --> from->b->f
        try { auto x = VnV::bfs(graph,"from","e");
            results["Two Level"] = (x.size()==2 && x[0].first== "b" && x[1].first == "e");
        } catch (...) {results["Two Level"]=false;}

        // Go from "from" to "h" --> from->b->f
        try { auto x = VnV::bfs(graph,"from","h");
            results["Three Level"] = (x.size()==3 && x[0].first== "b" && x[1].first == "e" && x[2].first == "h");
        } catch (...) {results["Three Level"]=false;}

        // Add in some loops
        std::map<std::string,std::string> ss4 = {{"h", "10"},{"b","3"},{"i","44"}};
        graph["h"] = ss4;
        try { auto x = VnV::bfs(graph,"from","i");
            results["Loops"] = (x.size()==4 && x[0].first== "b" && x[1].first == "e" && x[2].first == "h" && x[3].first == "i");
        } catch (...) {
            results["Loops"]=false;
        }


        return results;
    }

    ~BFSTester();
};

BFSTester::~BFSTester(){};

IUnitTester* BFSTester_maker() {
    return new BFSTester();
}

void BFSTester_callBack() {
    VnV::registerUnitTester("BFSTester ", BFSTester_maker);
}

#endif
