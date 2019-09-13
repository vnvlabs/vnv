
%.o : %.cpp 
	g++ -g -c -o $@ $< $(CPPFLAGS) -I. -I../../injection/build/include   
tester: test1.o test1a.o 
	ar -rcs libtester.a $^
.PHONY: clean


clean:
	rm -f *.o 
