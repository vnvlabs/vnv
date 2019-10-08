
VVDIR=../../../injection/

INCLUDES= -I.
INCLUDES+= -I../libraries/dummyLibOne/include
INCLUDES+= -I../libraries/dummyLibTwo/include
INCLUDES+= -I${VVDIR}/include

LIBS=  -L../libraries/dummyLibOne/ -ldummyLibOne
LIBS+= -L../libraries/dummyLibTwo/ -ldummyLibTwo 
LIBS+= -L${VVDIR} -linjection -Wl,-rpath,${VVDIR} 

.PHONY: all clean

all: sample 

clean:
	rm -f *.a 

sample : sample.cpp 
	g++ -g -o sample.a -rdynamic  ${INCLUDES} sample.cpp ${LIBS}

