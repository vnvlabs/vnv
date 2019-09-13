
VVDIR=../../injection/build/
include ${VVDIR}/config

INCLUDES=-I. -I../testLib_one -I../testLib_two ${VV_INCLUDES}
LIBS= -L../testLib_two -L../testLib_one -ltester -ltester_2 -ldl ${VV_LIBS} 

.PHONY: all clean

all: sample 

clean:
	rm -f *.a 

sample : sample.cpp 
	mpicxx -g -o sample.a -rdynamic  ${INCLUDES} sample.cpp ${LIBS}

