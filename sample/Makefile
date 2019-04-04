
CPPFLAGS=-isystem /home/boneill/llvm/adios2-INSTALL/include -std=gnu++11  

LDFLAGS=-Wl,-rpath,/home/boneill/llvm/adios2-INSTALL/lib /home/boneill/llvm/adios2-INSTALL/lib/libadios2.so.2.3.1 -pthread -Wl,-rpath-link,/home/boneill/llvm/adios2-INSTALL/lib -Wl,-rpath,/home/boneill/llvm/hdf5-install/lib /home/boneill/llvm/hdf5-install/lib/libhdf5.so  

VVDIR=/home/boneill/llvm/self-registering-injection-points/xml/vv_schema/build/

.PHONY: all clean

all: sample-xml sample 

clean:
	rm -f *.a 

sample-xml : sample-generate-xml.cpp 
	mpicxx -g -o sample-generate-xml.a -rdynamic  $(CPPFLAGS) -I. -I../injection/include -I ../testLib_two -I../testLib_one sample-generate-xml.cpp -Wl,-rpath,$(VVDIR)/lib -I $(VVDIR)/include -L$(VVDIR)/lib -L../injection/build -L../testLib_one/ -L../testLib_two -ltester_2 -ltester -linjection -lvv_schema -lxsdall -ldl $(LDFLAGS) -Wl,-rpath=/home/boneill/llvm/self-registering-injection-points/injection/build/



sample : sample.cpp 
	mpicxx -g -o sample.a -rdynamic  $(CPPFLAGS) -I. -I../injection/include -I ../testLib_two -I../testLib_one sample.cpp -Wl,-rpath,$(VVDIR)/lib -I $(VVDIR)/include -L$(VVDIR)/lib -L../injection/build -L../testLib_one/ -L../testLib_two -ltester_2 -ltester -linjection -lvv_schema -lxsdall -ldl $(LDFLAGS) -Wl,-rpath=/home/boneill/llvm/self-registering-injection-points/injection/build/
