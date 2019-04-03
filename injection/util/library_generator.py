
import argparse
from shutil import copyfile
import os


def generateMakefile(name):
    s ="INJECTION_INCLUDE=../injection/include\n"
    s+="SRC_DIR=./src\n"
    s+="LIBNAME=%s\n\n" %name 
    s+="CPPFLAGS=-fPIC -I $(INJECTION_INCLUDE)\n"
    s+="LDFLAGS=-shared\n"
    s+="SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)\n"
    s+="OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/%.o,$(SRC_FILES))\n"
    s+="all: $(OBJ_FILES)\n"
    s+="\tg++ $(LDFLAGS) -shared -o $(LIBNAME).so $^\n"
    s+="$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp\n"
    s+="\tg++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<\n"
    s+=".PHONY: clean\n"
    s+="clean:\n"
    s+="\trm $(OBJ_FILES) $(LIBNAME).so \n"
    return s

def generateLibrary(name, path):
    libPath = os.path.join(path,name)
    if not os.path.exists(libPath):
        os.makedirs(os.path.join(libPath,"src"))
        os.makedirs(os.path.join(libPath,"markdown"))
        real = os.path.dirname(os.path.realpath(__file__))
        copyfile(os.path.join(real,"test_generator.py"), os.path.join(os.path.join(libPath,"src"),"generate.py"))
    else:
        print "Cannot make library as library already exists;"
        return

    mname = os.path.join(name,"Makefile");
    with open(mname,"w") as f:
        f.write(generateMakefile(name))    

parser = argparse.ArgumentParser(description="Generate VV test Library Structure")
parser.add_argument('-n','--lname', help='The name of the Library', default='untitled')
parser.add_argument('-p','--lpath', help='The install path to the library', default="./")

args = vars(parser.parse_args())

libname = args["lname"]
libPath = args["lpath"]
print os.path.join(libPath,libname)
generateLibrary(libname,libPath)
