
# The number of arguments allowed in VV injection point descriptions. 
MAX_SUPPORTED_ARGUMENTS=25

XSD_INSTALL_DIR=/usr/local
ADIOS_INSTALL_DIR = /home/boneill/llvm/adios2-INSTALL
HDF5_DIR=/home/boneill/llvm/hdf5-install

CPPFLAGS=-isystem ${ADIOS_INSTALL_DIR}/include -std=gnu++11  
LDFLAGS= vv_schema_build/lib/libvv_schema.so ${XSD_INSTALL_DIR}/lib/libxsdall.so ${ADIOS_INSTALL_DIR}/lib/libadios2.so ${HDF5_DIR}/lib/libhdf5.so -pthread
LDFLAGS+= -Wl,-rpath-link,${ADIOS_INSTALL_DIR}/lib    
LDFLAGS+= -Wl,-rpath,${ADIOS_INSTALL_DIR}/lib    
LDFLAGS+= -Wl,-rpath,${PWD}/vv_schema_build/lib   
LDFLAGS+= -Wl,-rpath,${XSD_INSTALL_DIR}/lib
LDFLAGS+= -Wl,-rpath,${HDF5_DIR}/lib

XMLFLAGS= ${PWD}/vv_schema_build/include

%.o : %.cpp 
	g++ -shared -fPIC -g -c -o $@ $< -std=c++11 $(CPPFLAGS) -I./include -I$(XMLFLAGS)  

injection: src/injection.o
	g++ -fPIC -shared -o libinjection.so $^ $(LDFLAGS) 

install: injection
	mkdir -p build/lib
	cp libinjection.so ./build/lib
	cp -r include ./build/


.PHONY: generate-xml-api
generate-xml-api:
	${MAKE} -C xml 

.PHONY: generate-macros
generate-macros:
	python ./util/macro_generator.py ${MAX_SUPPORTED_ARGUMENTS}
	mv ./vv-runtime.h ./include/

.PHONY: clean
clean:
	rm -rf build/
	rm -f src/*.o 

