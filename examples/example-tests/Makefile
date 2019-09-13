



#### This is the makefile to build the shared library
INJECTION_INCLUDE=../injection/include 
SRC_DIR=./src
BUILD_DIR=./build
LIBNAME=testLib

CPPFLAGS= -std=c++11 -g -fPIC -I ../xml/vv_schema/build/include/ -I $(INJECTION_INCLUDE)
CPPFLAGS+= -I/home/boneill/llvm/self-registering-injection-points/injection/xml/vv_schema/include



LDFLAGS=-shared 

CPPFLAGS +=-isystem /home/boneill/llvm/adios2-INSTALL/include -std=gnu++11  
LDFLAGS +=-Wl,-rpath,/home/boneill/llvm/adios2-INSTALL/lib /home/boneill/llvm/adios2-INSTALL/lib/libadios2.so.2.3.1 -pthread -Wl,-rpath-link,/home/boneill/llvm/adios2-INSTALL/lib


SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/%.o,$(SRC_FILES))

all: $(OBJ_FILES)
	  g++ $(LDFLAGS) -shared -o $(BUILD_DIR)/$(LIBNAME).so $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	   g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(BUILD_DIR)/$(LIBNAME).so 
