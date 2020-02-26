#!/usr/bin/env python

from __future__ import print_function    # (at top of module)
import sys, os;
import subprocess


template = '''
#ifndef <Name>_H 
#define <Name>_H 

#include "VnV.h"
#include "interfaces/IUnitTester.h"

#include <sstream>
using namespace VnV;

class <Name> : public IUnitTester { 

public:

    <Name>( ){

    }

    std::map<std::string,bool> run(IOutputEngine* /* engine */) {
        
        std::map<std::string, bool> results;
        return results;
    }

    ~<Name>();
};

<Name>::~<Name>(){};

IUnitTester* <Name>_maker() {
    return new <Name>();
}

void <Name>_callBack() {
    VnV::registerUnitTester("<Name>", <Name>_maker);
}

#endif
'''

include_needle = "//<callback-include-needle>"
declare_needle = "//<callback-declare-needle>"
generate_needle ="//<callback-generation-needle>"
cmake_needle= "#<needle>"

def addToCmake(fname, runFile ):
    data = ""
    with open(runFile, "r") as f:
       data = f.read()
       with open(runFile + ".bak", "w") as ff:
           ff.write(data)

       dec = "target_sources(test-runner PRIVATE \"" + fname + "\")\n" + cmake_needle
       data = data.replace(cmake_needle, dec )
    if len(data) > 0:
      with open(runFile, "w") as f:
        f.write(data);

def removeFromCmake(fname, runFile, full):
    data = ""
    with open(runFile, "r") as f:
       data = f.read()
       with open(runFile + ".bak", "w") as ff:
           ff.write(data)

       dec = "target_sources(test-runner PRIVATE \"" + fname + "\")" 
       data = data.replace(dec, (("#" + dec) if not full else "" ))
    if len(data) > 0:
      with open(runFile, "w") as f:
        f.write(data);

def addToRunner(runFile, name, fname ):
    data = ""
    with open(runFile, "r") as f:
       data = f.read()
       with open(runFile + ".bak", "w") as ff:
           ff.write(data)

       dec = "void " + name + "_callBack(); \n" + declare_needle
       gen = name + "_callBack(); \n    " + generate_needle
       data = data.replace(declare_needle, dec )
       data = data.replace(generate_needle, gen )
    if len(data) > 0:
      with open(runFile, "w") as f:
        f.write(data);

def removeFromRunner(runFile, name, fname, full ):
    data = ""
    with open(runFile, "r") as f:
       data = f.read()
       with open(runFile + ".bak", "w") as ff:
           ff.write(data)

       dec = "void " + name + "_callBack();" 
       gen = name + "_callBack();"
       data = data.replace(dec, ("//"+dec) if not full else "" )
       data = data.replace(gen, ("//"+gen) if not full else "" )
    if len(data) > 0:
      with open(runFile, "w") as f:
        f.write(data);


if len(sys.argv) == 2:
    name = sys.argv[1]
    fname = "./unit-testers/" + name + ".cpp"
    if os.path.exists(fname):
        sys.exit("Test suite with that name already exists")

    with open(fname, "w") as f:
        f.write(template.replace("<Name>",name))
    os.system("git add " + fname) 

    addToRunner("./test-runner.cpp", name, fname)
    addToCmake(fname, "./CMakeLists.txt")

elif len(sys.argv) == 3 and (sys.argv[1] == "--r" ) :
    name = sys.argv[2]
    fname = "./unit-testers/" + name + ".cpp"
    if not os.path.exists(fname):
        sys.exit("Test suite with that name doesn't exist")
    removeFromCmake(fname, "./CMakeLists.txt", True )
    removeFromRunner("./test-runner.cpp", name, fname, True)
    if True:
       os.remove(fname)
    else: 
       os.rename(fname, fname + ".removed")
    os.system("git rm " + fname)
else:
  sys.exit("Invalid Usage: either <exe> <Name> to generate <Name>.cpp or <exe> --r <Name> to remove a suite")
