



#### This is the makefile to build the shared library
INJECTION_INCLUDE=../../../../injection/include 
SRC_DIR=./src
LIBNAME=testLib

CPPFLAGS= -std=c++11 -g -fPIC -I$(INJECTION_INCLUDE)
LDFLAGS=-shared 

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(SRC_DIR)/%.o,$(SRC_FILES))

all: $(OBJ_FILES)
	  mpicxx $(LDFLAGS) -shared -o $(LIBNAME).so $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	   g++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f $(OBJ_FILES) $(BUILD_DIR)/$(LIBNAME).so 
