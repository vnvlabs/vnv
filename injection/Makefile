
CPPFLAGS=-isystem /home/boneill/llvm/adios2-install/include -std=gnu++11  

LDFLAGS=-Wl,-rpath,/home/boneill/llvm/adios2-INSTALL/lib /home/boneill/llvm/adios2-INSTALL/lib/libadios2.so.2.3.1 -pthread -Wl,-rpath-link,/home/boneill/llvm/adios2-INSTALL/lib 

XMLFLAGS=/home/boneill/llvm/self-registering-injection-points/injection/xml/vv_schema/build/include

%.o : %.cpp 
	g++ -shared -fPIC -g -c -o $@ $< -std=c++11 $(CPPFLAGS) -I./include -I$(XMLFLAGS)  

injection: src/injection.o
	g++ -fPIC -shared -o build/libinjection.so $^ $(LDFLAGS) 

.PHONY: clean
clean:
	rm -f build/*
	rm -f src/*.o 

