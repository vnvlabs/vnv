
CPPFLAGS=-isystem /home/boneill/llvm/adios2-install/include -std=gnu++11  

LDFLAGS=-Wl,-rpath,/home/boneill/llvm/adios2-INSTALL/lib /home/boneill/llvm/adios2-INSTALL/lib/libadios2.so.2.3.1 -pthread -Wl,-rpath-link,/home/boneill/llvm/adios2-INSTALL/lib 



%.o : %.cpp 
	g++ -shared -fPIC -g -c -o $@ $< -std=c++11 $(CPPFLAGS) -I ./include -I /usr/local/include/	-I ../xml/vv_schema/build/include  

injection: src/injection.o
	g++ -fPIC -shared -o build/libinjection.so $^ $(LDFLAGS) 

.PHONY: clean
clean:
	rm build/*
	rm src/*.o 

