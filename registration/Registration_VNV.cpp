/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME VNV
#include "VnV.h" 
DECLARETEST(hardBreakpoint)
DECLARETEST(printMessage)
DECLARETEST(provenance)
DECLARETEST(test1)
DECLAREENGINE(adios)
DECLAREENGINE(debug)
DECLAREENGINE(json)
DECLAREUNITTEST(BFSTester)
DECLAREUNITTEST(Demo)
DECLAREUNITTEST(Sample2)
DECLARESERIALIZER(doubleToString)
DECLARETRANSFORM(doubleToInt)
DECLAREOPTIONS
const char* getFullRegistrationJson_VNV(){
	 return "{\"Engines\":{\"adios\":{\"docs\":\"\"},\"debug\":{\"docs\":\"\"},\"json\":{\"docs\":\"\n\n  This is the documentation.\"}},\"InjectionPoints\":{\"initialization\":{\"name\":\"initialization\",\"packageName\":\"VNV\",\"parameters\":[{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}],\"stages\":{\"Begin\":{\"docs\":\"\n Injection point documentation.\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":259,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":72}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"Finalize\",\"Calling Function Column\":1,\"Calling Function Line\":332,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":79}}}}},\"Options\":{\"docs\":\"\"},\"Serializers\":{\"doubleToString\":{\"docs\":\"\",\"type\":\"double\"}},\"Tests\":{\"hardBreakpoint\":{\"docs\":\"\",\"parameters\":{\"argv\":\"int*\"}},\"printMessage\":{\"docs\":\"\n  PRINT TEST: The message is TODO.\",\"parameters\":{}},\"provenance\":{\"docs\":\"sdfsdfsdfsdf *\",\"parameters\":{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}},\"test1\":{\"docs\":\"\n @brief INJECTION_TEST\n @param argv\n @param argc\n @param config\n\n This is the way\n \",\"parameters\":{\"argc\":\"char**\",\"argv\":\"int\",\"config\":\"std::string\"}}},\"Transforms\":{\"doubleToInt\":{\"docs\":\"\",\"from\":\"int\",\"to\":\"double\"}},\"UnitTests\":{\"BFSTester\":{\"docs\":\"\"},\"Demo\":{\"docs\":\"\"},\"Sample2\":{\"docs\":\"\n  A sample unit test.\"}}}";}

INJECTION_REGISTRATION(){
	REGISTERTEST(hardBreakpoint);
	REGISTERTEST(printMessage);
	REGISTERTEST(provenance);
	REGISTERTEST(test1);
	REGISTERENGINE(adios);
	REGISTERENGINE(debug);
	REGISTERENGINE(json);
	REGISTERUNITTEST(BFSTester);
	REGISTERUNITTEST(Demo);
	REGISTERUNITTEST(Sample2);
	REGISTERSERIALIZER(doubleToString);
	REGISTERTRANSFORM(doubleToInt);
	REGISTEROPTIONS
	Register_Injection_Point(initialization,"{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}");
	REGISTER_FULL_JSON(getFullRegistrationJson_VNV);
};



