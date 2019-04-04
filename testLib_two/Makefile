CPPFLAGS=-isystem /home/boneill/llvm/adios2-INSTALL/include -std=gnu++11  
LDFLAGS=-Wl,-rpath,/home/boneill/llvm/adios2-INSTALL/lib /home/boneill/llvm/adios2-INSTALL/lib/libadios2.so.2.3.1 -pthread -Wl,-rpath-link,/home/boneill/llvm/adios2-INSTALL/lib



%.o : %.c 
	g++ -g -c -o $@ $< $(CPPFLAGS) -I. -I../injection/include -I../xml/vv_schema/build/include  

tester: test.o 
	ar -rcs libtester_2.a $^
.PHONY: clean


clean:
	rm -f *.o 
