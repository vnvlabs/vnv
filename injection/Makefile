
# The number of arguments allowed in VV injection point descriptions. 
MAX_SUPPORTED_ARGUMENTS=25

XSD_INSTALL_DIR=/usr/local

CPPFLAGS=-I./include -I./xml/vv_schema/build/include -std=gnu++11  
LDFLAGS= ${PWD}/xml/vv_schema/build/lib/libvv_schema.so ${XSD_INSTALL_DIR}/lib/libxsdall.so -pthread
LDFLAGS+= -Wl,-rpath,${PWD}/xml/vv_schema/build/lib   
LDFLAGS+= -Wl,-rpath,${XSD_INSTALL_DIR}/lib

SRCS=src/injection.o src/vv-testing.o src/vv-output.o src/vv-adios.o

WITH_ADIOS2=1
ADIOS2_DIR?=/home/boneill/llvm/adios2-INSTALL
ifeq ($(WITH_ADIOS2),1)
CPPFLAGS+=-isystem ${ADIOS2_DIR}/include
LDFLAGS+=-Wl,-rpath,${ADIOS2_DIR}/lib ${ADIOS2_DIR}/lib/libadios2.so.2.3.1 -lpthread -Wl,-rpath-link,${ADIOS2_DIR}/lib 
endif

%.o : %.cpp 
	mpicxx -shared -fPIC -g -c -o $@ $< -std=c++11 $(CPPFLAGS) -I./include   

injection: ${SRCS}
	mpicxx -fPIC -shared -o libinjection.so $^ $(LDFLAGS) 

install: injection
	mkdir -p build/lib
	cp libinjection.so ./build/lib
	cp -r include ./build/


.PHONY: generate-xml-api
generate-xml-api: xml/vv_schema.xsd 
	cd xml && ${XSD_INSTALL_DIR}/bin/xsd2cpp vv_schema.xsd && cd vv_schema && ./autogen.sh && make && make install


.PHONY: clean
clean:
	rm -rf build/
	rm -f src/*.o 

