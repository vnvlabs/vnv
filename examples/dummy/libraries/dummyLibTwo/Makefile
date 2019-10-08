
%.o : %.cpp 
	g++ -g -c -o $@ $< $(CPPFLAGS) -I./include -I../../../../injection/include   
tester: src/class1.o src/class2.o 
	ar -rcs libdummyLibTwo.a $^
.PHONY: clean


clean:
	rm -f *.a src/*.o 
