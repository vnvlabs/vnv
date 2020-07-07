/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME VNV
#include "VnV.h" 
DECLARETEST(cputime)
DECLARETEST(hardBreakpoint)
DECLARETEST(printMessage)
DECLARETEST(provenance)
DECLARETEST(test1)
DECLAREENGINE(Parallel)
DECLAREENGINE(adios)
DECLAREENGINE(debug)
DECLAREENGINE(json)
DECLAREUNITTEST(Sample2)
DECLARESERIALIZER(doubleToString)
DECLARETRANSFORM(doubleToInt)
DECLARECOMM(mpi)
DECLARECOMM(serial)
DECLAREOPTIONS
const char* getFullRegistrationJson_VNV(){
	 return "{\"Comms\":{\"mpi\":{\"docs\":\"\"},\"serial\":{\"docs\":\"\\n  A Serial communicator that implements the Comm interface for single core runs.\"}},\"Engines\":{\"Parallel\":{\"docs\":\"\"},\"adios\":{\"docs\":\"\"},\"debug\":{\"docs\":\"\"},\"json\":{\"docs\":\"\\n\\n  This is the documentation.\"}},\"InjectionPoints\":{\"configuration\":{\"docs\":\"\",\"name\":\"configuration\",\"packageName\":\"VNV\",\"parameters\":[{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":318,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":78}}}},\"initialization\":{\"docs\":\"\\n VnV Application Profiling Loop.\\n ===============================\\n\\n This injection point is called at the end of the VnVInit function. This is a looped\\n injection point with no interesting parameters passed in. This injection point exists\\n soley as a mechanism for profiling the given application between the VnVInit and VnVFinalize\\n functions.\\n\\n    \",\"name\":\"initialization\",\"packageName\":\"VNV\",\"parameters\":[{\"runTests\":\"_Bool\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":318,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":88}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"Finalize\",\"Calling Function Column\":1,\"Calling Function Line\":445,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":97}}}}},\"Options\":{\"docs\":\"\"},\"Serializers\":{\"doubleToString\":{\"docs\":\"\",\"type\":\"double\"}},\"Tests\":{\"cputime\":{\"docs\":\"\\n Injection Point Timing Results\\n ==============================\\n\\n The overall time was :vnv:`$.data.TotalTime` :vnv:`$.data.units`\\n\\n .. vnv-chart::\\n    :labels: $.data[\?TypeStr == \\\"Double\\\"].Name\\n    :ydata: $.data[\?TypeStr == \\\"Double\\\"].Value\\n\\n    {\\n       \\\"type\\\" : \\\"line\\\",\\n       \\\"data\\\" : {\\n          \\\"labels\\\" : $$labels$$,\\n          \\\"datasets\\\" : [{\\n             \\\"label\\\": \\\"Recorded CPU Times\\\",\\n             \\\"backgroundColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"borderColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"data\\\": $$ydata$$\\n           }]\\n       }\\n    }\\n\\n \",\"parameters\":{}},\"hardBreakpoint\":{\"docs\":\"\",\"parameters\":{\"argv\":\"int*\"}},\"printMessage\":{\"docs\":\"\\n  PRINT TEST: The message is TODO.\",\"parameters\":{}},\"provenance\":{\"docs\":\"sdfsdfsdfsdf *\",\"parameters\":{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}},\"test1\":{\"docs\":\"\\n @brief INJECTION_TEST\\n @param argv\\n @param argc\\n @param config\\n\\n This is the way\\n \",\"parameters\":{\"argc\":\"char**\",\"argv\":\"int\",\"config\":\"std::string\"}}},\"Transforms\":{\"doubleToInt\":{\"docs\":\"\",\"from\":\"int\",\"to\":\"double\"}},\"UnitTests\":{\"Sample2\":{\"docs\":\"\\n  A sample unit test.\"}}}";}

INJECTION_REGISTRATION(){
	REGISTERTEST(cputime);
	REGISTERTEST(hardBreakpoint);
	REGISTERTEST(printMessage);
	REGISTERTEST(provenance);
	REGISTERTEST(test1);
	REGISTERENGINE(Parallel);
	REGISTERENGINE(adios);
	REGISTERENGINE(debug);
	REGISTERENGINE(json);
	REGISTERUNITTEST(Sample2);
	REGISTERSERIALIZER(doubleToString);
	REGISTERTRANSFORM(doubleToInt);
	REGISTERCOMM(mpi);
	REGISTERCOMM(serial);
	REGISTEROPTIONS
	Register_Injection_Point(configuration,"{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}");
	Register_Injection_Point(initialization,"{\"runTests\":\"_Bool\"}");
	REGISTER_FULL_JSON(getFullRegistrationJson_VNV);
};



