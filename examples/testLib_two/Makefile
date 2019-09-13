


%.o : %.c 
	g++ -g -c -o $@ $< $(CPPFLAGS) -I. -I../../injection/build/include   

tester: test.o 
	ar -rcs libtester_2.a $^
.PHONY: clean


clean:
	rm -f *.o 
