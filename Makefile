

VV_INJECTION_INSTALL_DIR=/home/boneill/software


all: xplus vv_schema injection examples 

.PHONY: xplus 
xplus: 
	${MAKE} -c xplus-xsd2cpp


vv_schema: vv_schema



