///9287212607282021480
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: VNV

#include "VnV.h" 
DECLARETEST(VNV,cputime)
DECLARETEST(VNV,hardBreakpoint)
DECLARETEST(VNV,printMessage)
DECLARETEST(VNV,provenance)
DECLARETEST(VNV,test1)
DECLAREENGINE(VNV,Parallel)
DECLAREENGINE(VNV,adios)
DECLAREENGINE(VNV,debug)
DECLAREENGINE(VNV,json)
DECLAREUNITTEST(VNV,Sample2)
DECLARESERIALIZER(VNV,doubleToString)
DECLARETRANSFORM(VNV,doubleToInt)
DECLARECOMM(VNV,mpi)
DECLARECOMM(VNV,serial)
DECLAREOPTIONS(VNV)
const char* getFullRegistrationJson_VNV(){
	 return "{\"Comms\":{\"mpi\":{\"docs\":\"\"},\"serial\":{\"docs\":\"\\n  A Serial communicator that implements the Comm interface for single core runs.\"}},\"Engines\":{\"Parallel\":{\"docs\":\"\"},\"adios\":{\"docs\":\"\"},\"debug\":{\"docs\":\"\"},\"json\":{\"docs\":\"\\n\\n  This is the documentation.\"}},\"InjectionPoints\":{\"configuration\":{\"docs\":\"\\n VnV Configuration and provenance Tracking information.\\n ======================================================\\n\\n CmdLine:  :vnv:`Data.Data.\\\"command-line\\\"`.\\n\\n Time of execution :vnv:`Data.Data.time`.\\n\\n The VnV Configuration file was\\n\\n .. vnv-jchart::\\n   :main: Data.Data.config.Value\\n\\n   $$main$$\\n\\n\\n This injection point is called at the end of the VnVInit function\\n to allow users to collect provenance information about the executable.\\n\\n   \",\"name\":\"configuration\",\"packageName\":\"VNV\",\"parameters\":[{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":316,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":79}}}},\"initialization\":{\"docs\":\"\\nVnV Application Profiling Loop.\\n===============================\\n\\nThis injection point is called at the end of the VnVInit function. This is a\\nlooped injection point with no interesting parameters passed in. This\\ninjection point exists soley as a mechanism for profiling the given\\napplication between the VnVInit and VnVFinalize functions.\\n\\n   \",\"name\":\"initialization\",\"packageName\":\"VNV\",\"parameters\":[{\"runTests\":\"_Bool\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":316,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":89}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"Finalize\",\"Calling Function Column\":1,\"Calling Function Line\":446,\"filename\":\"/home/ben/wfr/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":98}}}}},\"Options\":\"\",\"Serializers\":{\"doubleToString\":{\"docs\":\"\",\"type\":\"double\"}},\"Tests\":{\"cputime\":{\"docs\":\"\\n Injection Point Timing Results\\n ==============================\\n\\n The overall time was :vnv:`Data.TotalTime` :vnv:`Data.units`\\n\\n .. vnv-chart::\\n    :labels: Data[\?TypeStr == \'Double\'].Name\\n    :ydata: Data[\?TypeStr == \'Double\'].Value\\n\\n    {\\n       \\\"type\\\" : \\\"line\\\",\\n       \\\"data\\\" : {\\n          \\\"labels\\\" : $$labels$$,\\n          \\\"datasets\\\" : [{\\n             \\\"label\\\": \\\"Recorded CPU Times\\\",\\n             \\\"backgroundColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"borderColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"data\\\": $$ydata$$\\n           }]\\n       }\\n    }\\n\\n \",\"parameters\":{}},\"hardBreakpoint\":{\"docs\":\"\",\"parameters\":{\"argv\":\"int*\"}},\"printMessage\":{\"docs\":\"\\n  .. (comment) vnv-process is the best way to handle nested parsing.\\n     Basically \\\"mess\\\" will be substituted in for $$mess$$. This\\n     allows for a message that contains restructured text to be\\n     processed properly.\\n\\n  .. vnv-process::\\n    :mess: `Data.message`\\n\\n    $$mess$$\",\"parameters\":{}},\"provenance\":{\"docs\":\"\\n    Provenance Tracking\\n    ===================\\n\\n    In this section, we detail the provenance tracking information for the\\n    sample executable. This includes a robust, fully detailed description of\\n    the software used to compile and run this code. This includes a detailed\\n    account of all shared libraries loaded in the application, as well as the\\n    time of execution, the command line used and the vnv configuration file.\\n\\n    The configuration is:\\n\\n    Current Working directory: :vnv:`Data.cwd.Value`\\n\\n    Command Line: :vnv:`Data.\\\"command-line\\\".Value`\\n\\n    Time: :vnv:`Data.time.Value`\\n\\n    The runtime config info was:\\n\\n    .. vnv-jchart::\\n       :exe: Data.\\\"exe-info\\\".Value\\n       :inp: Data.\\\"input-files\\\".Value\\n       :conf: Data.\\\"vnv-config\\\".Value\\n       :time: Data.time.Value\\n       :cmd: Data.\\\"command-line\\\".Value\\n       :cwd: Data.cwd.Value\\n\\n       {\\n         \\\"Command Line\\\" : $$cmd$$,\\n         \\\"Working Directory\\\" : $$cwd$$,\\n         \\\"time\\\" : $$time$$,\\n         \\\"configuration File\\\" : $$conf$$,\\n         \\\"input Files\\\" : $$inp$$,\\n         \\\"exe info\\\" : $$exe$$\\n       }\\n\\n\\n\\n    TODO Represent this data in a nicer, less brute force way\\n \",\"parameters\":{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}},\"test1\":{\"docs\":\"\\n @brief INJECTION_TEST\\n @param argv\\n @param argc\\n @param config\\n\\n This is the way\\n \",\"parameters\":{\"argc\":\"char**\",\"argv\":\"int\",\"config\":\"std::string\"}}},\"Transforms\":{\"doubleToInt\":{\"docs\":\"\",\"from\":\"int\",\"to\":\"double\"}},\"UnitTests\":{\"Sample2\":{\"docs\":\"\\n  A sample unit test.\",\"tests\":{\"false\":\"\",\"not true\":\"\",\"true\":\"\"}}}}";}

INJECTION_REGISTRATION(VNV){
	REGISTERTEST(VNV,cputime);
	REGISTERTEST(VNV,hardBreakpoint);
	REGISTERTEST(VNV,printMessage);
	REGISTERTEST(VNV,provenance);
	REGISTERTEST(VNV,test1);
	REGISTERENGINE(VNV,Parallel);
	REGISTERENGINE(VNV,adios);
	REGISTERENGINE(VNV,debug);
	REGISTERENGINE(VNV,json);
	REGISTERUNITTEST(VNV,Sample2);
	REGISTERSERIALIZER(VNV,doubleToString);
	REGISTERTRANSFORM(VNV,doubleToInt);
	REGISTERCOMM(VNV,mpi);
	REGISTERCOMM(VNV,serial);
	REGISTEROPTIONS(VNV)
	Register_Injection_Point(VNV,configuration,"{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}");
	Register_Injection_Point(VNV,initialization,"{\"runTests\":\"_Bool\"}");
	REGISTER_FULL_JSON(VNV, getFullRegistrationJson_VNV);
};



