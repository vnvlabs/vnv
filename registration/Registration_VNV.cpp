///3614320580714881338
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
DECLARETEST(VNV,vectorDump)
DECLAREITERATOR(VNV,singleParameterMinimization)
DECLAREENGINE(VNV,Parallel)
DECLAREENGINE(VNV,adios)
DECLAREENGINE(VNV,debug)
DECLAREENGINE(VNV,json)
DECLAREUNITTEST(VNV,CommMapTester)
DECLAREUNITTEST(VNV,CommTestTwoProc)
DECLAREUNITTEST(VNV,Sample2)
DECLARESERIALIZER(VNV,doubleToString)
DECLARETRANSFORM(VNV,doubleToInt)
DECLARECOMM(VNV,mpi)
DECLARECOMM(VNV,serial)
DECLAREREDUCER(VNV,max)
DECLAREREDUCER(VNV,min)
DECLAREREDUCER(VNV,prod)
DECLAREREDUCER(VNV,sqrsum)
DECLAREREDUCER(VNV,sum)
DECLAREDATATYPE(VNV,double)
DECLAREDATATYPE(VNV,float)
DECLAREDATATYPE(VNV,int)
DECLAREDATATYPE(VNV,long)
DECLAREDATATYPE(VNV,longlong)
DECLAREDATATYPE(VNV,short)
DECLAREDATATYPE(VNV,string)
DECLAREOPTIONS(VNV)
const char* getFullRegistrationJson_VNV(){
	 return "{\"Comms\":{\"mpi\":{\"docs\":\"\"},\"serial\":{\"docs\":\"\\n  A Serial communicator that implements the Comm interface for single core runs.\"}},\"DataTypes\":{\"double\":{\"docs\":\"\"},\"float\":{\"docs\":\"\"},\"int\":{\"docs\":\"\"},\"long\":{\"docs\":\"\"},\"longlong\":{\"docs\":\"\"},\"short\":{\"docs\":\"\"},\"string\":{\"docs\":\"\"}},\"Engines\":{\"Parallel\":{\"docs\":\"\"},\"adios\":{\"docs\":\"\"},\"debug\":{\"docs\":\"\"},\"json\":{\"docs\":\"\\n\\n  This is the documentation.\"}},\"InjectionPoints\":{\"configuration\":{\"docs\":\"\\n VnV Configuration and provenance Tracking information.\\n ======================================================\\n\\n CmdLine:  :vnv:`Data.Data.\\\"command-line\\\"`.\\n\\n Time of execution :vnv:`Data.Data.time`.\\n The VnV Configuration file was\\n\\n .. vnv-jchart::\\n   :main: Data.Data.config.Value\\n\\n   $$main$$\\n\\n\\n This injection point is called at the end of the VnVInit function\\n to allow users to collect provenance information about the executable.\\n\\n   \",\"name\":\"configuration\",\"packageName\":\"VNV\",\"parameters\":[{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":409,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":120}}}},\"initialization\":{\"docs\":\"\\nVnV Application Profiling Loop.\\n===============================\\n\\nThis injection point is called at the end of the VnVInit function. This is a\\nlooped injection point with no interesting parameters passed in. This\\ninjection point exists soley as a mechanism for profiling the given\\napplication between the VnVInit and VnVFinalize functions.\\n\\n   \",\"name\":\"initialization\",\"packageName\":\"VNV\",\"parameters\":[{\"runTests\":\"_Bool\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"InitFromJson\",\"Calling Function Column\":1,\"Calling Function Line\":409,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":130}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"Finalize\",\"Calling Function Column\":1,\"Calling Function Line\":541,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":139}},\"cp\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"injectionPoint_begin\",\"Calling Function Column\":1,\"Calling Function Line\":255,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":143}},\"cpi\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"injectionPoint\",\"Calling Function Column\":1,\"Calling Function Line\":155,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":143}},\"cpp\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"injectionPoint\",\"Calling Function Column\":1,\"Calling Function Line\":242,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":143}},\"cppi\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"injectionPoint_begin\",\"Calling Function Column\":1,\"Calling Function Line\":228,\"filename\":\"/home/ben/work/vnv/vv-neams/injection/src/base/Runtime.cpp\",\"lineColumn\":5,\"lineNumber\":143}}}}},\"Iterators\":{\"singleParameterMinimization\":{\"docs\":\"\\n\\n   VnV Linear parameter search\\n   ===========================\\n\\n   The optimal values for this function are [:vnv:`data.xopt` , :vnv:`data.yopt` ].\",\"parameters\":{}}},\"Options\":\"\",\"Reducers\":{\"max\":{\"docs\":\"\"},\"min\":{\"docs\":\"\"},\"prod\":{\"docs\":\"\"},\"sqrsum\":{\"docs\":\"\"},\"sum\":{\"docs\":\"\"}},\"Serializers\":{\"doubleToString\":{\"docs\":\"\",\"type\":\"double\"}},\"Tests\":{\"cputime\":{\"docs\":\"\\n Injection Point Timing Results\\n ==============================\\n\\n The overall time was :vnv:`Data.TotalTime` :vnv:`Data.units`\\n\\n .. vnv-chart::\\n    :labels: Data[\?TypeStr == \'Double\'].Name\\n    :ydata: Data[\?MetaData.tag == \'value\'].Value\\n\\n    {\\n       \\\"type\\\" : \\\"line\\\",\\n       \\\"data\\\" : {\\n          \\\"labels\\\" : $$labels$$,\\n          \\\"datasets\\\" : [{\\n             \\\"label\\\": \\\"Recorded CPU Times\\\",\\n             \\\"backgroundColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"borderColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"data\\\": $$ydata$$\\n           }]\\n       },\\n       \\\"options\\\" : {\\n           \\\"responsive\\\" : true,\\n           \\\"title\\\" : { \\\"display\\\" : true, \\\"text\\\" : \\\"CPU Time at the begining of each injection point.\\\" },\\n           \\\"scales\\\": {\\n               \\\"yAxes\\\": [{\\n                   \\\"scaleLabel\\\": {\\n                       \\\"display\\\": true,\\n                       \\\"labelString\\\": \\\"CPU Time (nanoseconds)\\\"\\n                   }\\n               }],\\n               \\\"xAxes\\\": [{\\n                   \\\"scaleLabel\\\": {\\n                       \\\"display\\\":true,\\n                       \\\"labelString\\\": \\\"Injection Point Stage\\\"\\n                   }\\n               }]\\n            }\\n        }\\n    }\\n\\n \",\"parameters\":{}},\"hardBreakpoint\":{\"docs\":\"\",\"parameters\":{\"argv\":\"int*\"}},\"printMessage\":{\"docs\":\"\\n  .. (comment) vnv-process is the best way to handle nested parsing.\\n     Basically \\\"mess\\\" will be substituted in for $$mess$$. This\\n     allows for a message that contains restructured text to be\\n     processed properly.\\n\\n  .. vnv-process::\\n    :mess: `Data.message`\\n\\n    $$mess$$\",\"parameters\":{}},\"provenance\":{\"docs\":\"\\n    Provenance Tracking\\n    ===================\\n\\n    In this section, we detail the provenance tracking information for the\\n    sample executable. This includes a robust, fully detailed description of\\n    the software used to compile and run this code. This includes a detailed\\n    account of all shared libraries loaded in the application, as well as the\\n    time of execution, the command line used and the vnv configuration file.\\n\\n    The configuration is:\\n\\n    Current Working directory: :vnv:`Data.cwd.Value`\\n\\n    Command Line: :vnv:`Data.\\\"command-line\\\".Value`\\n\\n    Time: :vnv:`Data.time.Value`\\n\\n    The runtime config info was:\\n\\n    .. vnv-jchart::\\n       :exe: Data.\\\"exe-info\\\".Value\\n       :inp: Data.\\\"input-files\\\".Value\\n       :conf: Data.\\\"vnv-config\\\".Value\\n       :time: Data.time.Value\\n       :cmd: Data.\\\"command-line\\\".Value\\n       :cwd: Data.cwd.Value\\n\\n       {\\n         \\\"Command Line\\\" : $$cmd$$,\\n         \\\"Working Directory\\\" : $$cwd$$,\\n         \\\"time\\\" : $$time$$,\\n         \\\"configuration File\\\" : $$conf$$,\\n         \\\"input Files\\\" : $$inp$$,\\n         \\\"exe info\\\" : $$exe$$\\n       }\\n\\n\\n\\n    TODO Represent this data in a nicer, less brute force way\\n \",\"parameters\":{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}},\"vectorDump\":{\"docs\":\"\\n Test Writing a Vector to File\\n =============================\\n\\n The global size of the vector was :vnv:`len(Data.result)` `\\n\\n .. vnv-chart::\\n    :labels: Data.result.Children.Name\\n    :ydata: Data.result.Children.Value\\n\\n    {\\n       \\\"type\\\" : \\\"line\\\",\\n       \\\"data\\\" : {\\n          \\\"labels\\\" : $$labels$$,\\n          \\\"datasets\\\" : [{\\n             \\\"label\\\": \\\"Random information\\\",\\n             \\\"backgroundColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"borderColor\\\": \\\"rgb(255, 99, 132)\\\",\\n             \\\"data\\\": $$ydata$$\\n           }]\\n       }\\n    }\",\"parameters\":{}}},\"Transforms\":{\"doubleToInt\":{\"docs\":\"\",\"from\":\"int\",\"to\":\"double\"}},\"UnitTests\":{\"CommMapTester\":{\"docs\":\"\\n Communication Mapper Testing\\n ============================\\n\\n The CommMapper class is included in the VnV toolkit as a built in way to track\\n communication patterns in parallel applications.\\n\\n \",\"tests\":{\"Add Same Comm Twice\":\"\\n Adding the same comm again should do nothing. This is all but guaranteed assuming comm\\n returns the same id.\\n     \",\"AddComm\":\"\\n After Adding a communicator was the comms set property. This test\\n ensures that logging a comm adds it to the list of registered comms.\\n\\n     \",\"Compiles\":\"\\n Does the comm-map tester compile. This test never fails.\\n    \",\"Constructor\":\"\\n Does Constructor work\\n    \",\"God Test Two\":\"\\n Test 2:\\n -------\\n\\n Processor 0: World, Self, {0,1}, {0,2}\\n Processor 1: World, Self, {0,1}, {1,2}\\n Processor 2: World, Self, {0,2}, {1,2}\\n\\n The expected Result is:\\n\\n .. vnv-jchart::\\n\\n   {\\\"0\\\":{\\\"c\\\":[0],\\\"ch\\\":[],\\\"p\\\":[10,20]},\\\"1\\\":{\\\"c\\\":[1],\\\"ch\\\":[],\\\"p\\\":[10,30]},\\\"10\\\":{\\\"c\\\":[0,1],\\\"ch\\\":[0,1],\\\"p\\\":[100]},\\\"100\\\":{\\\"c\\\":[0,1,2],\\\"ch\\\":[10,20,30],\\\"p\\\":[]},\\\"2\\\":{\\\"c\\\":[2],\\\"ch\\\":[],\\\"p\\\":[20,30]},\\\"20\\\":{\\\"c\\\":[0,2],\\\"ch\\\":[0,2],\\\"p\\\":[100]},\\\"30\\\":{\\\"c\\\":[1,2],\\\"ch\\\":[1,2],\\\"p\\\":[100]}}\\n\\n     \",\"Gold Test Four\":\"\\n Test 4:\\n\\n Processor 0-5: World, SELF\\n\\n The expected result is:\\n\\n .. vnv-jchart::\\n\\n  {\\\"100\\\": {\\\"c\\\":[0,1,2,3,4,5], \\\"ch\\\":[0,1,2,3,4,5], \\\"p\\\":[]},\\\"0\\\":{\\\"c\\\":[0], \\\"ch\\\":[],\\\"p\\\":[100]},\\\"1\\\":{\\\"c\\\":[1], \\\"ch\\\":[],\\\"p\\\":[100]},\\\"2\\\":{\\\"c\\\":[2], \\\"ch\\\":[],\\\"p\\\":[100]},\\\"3\\\":{\\\"c\\\":[3], \\\"ch\\\":[],\\\"p\\\":[100]},\\\"4\\\":{\\\"c\\\":[4], \\\"ch\\\":[],\\\"p\\\":[100]},\\\"5\\\":{\\\"c\\\":[5], \\\"ch\\\":[],\\\"p\\\":[100]}}\\n\\n     \",\"Gold Test One\":\"\\n Test 1:\\n -------\\n\\n Processor 0: World, Self\\n Processor 1: World, Self\\n\\n The expected Result is:\\n\\n .. vnv-jchart::\\n\\n    {\\\"0\\\":{\\\"c\\\":[0],\\\"ch\\\":[],\\\"p\\\":[10]},\\\"1\\\":{\\\"c\\\":[1],\\\"ch\\\":[],\\\"p\\\":[10]},\\\"10\\\":{\\\"c\\\":[0,1],\\\"ch\\\":[0,1],\\\"p\\\":[]}}\\n\\n     \",\"Gold Test Three\":\"\\n Test 3:\\n\\n Processor 0: World\\n\\n The expected result is:\\n\\n .. vnv-jchart::\\n\\n   {\\\"0\\\":{\\\"c\\\":[0], \\\"ch\\\":[],\\\"p\\\":[]}}\\n\\n     \"}},\"CommTestTwoProc\":{\"docs\":\"\",\"tests\":{\"Full Map Test\":\"\"}},\"Sample2\":{\"docs\":\"\\n  A sample unit test.\",\"tests\":{\"false\":\"\",\"not true\":\"\",\"true\":\"\"}}}}";}

INJECTION_REGISTRATION(VNV){
	REGISTERTEST(VNV,cputime);
	REGISTERTEST(VNV,hardBreakpoint);
	REGISTERTEST(VNV,printMessage);
	REGISTERTEST(VNV,provenance);
	REGISTERTEST(VNV,vectorDump);
	REGISTERITERATOR(VNV,singleParameterMinimization);
	REGISTERENGINE(VNV,Parallel);
	REGISTERENGINE(VNV,adios);
	REGISTERENGINE(VNV,debug);
	REGISTERENGINE(VNV,json);
	REGISTERUNITTEST(VNV,CommMapTester);
	REGISTERUNITTEST(VNV,CommTestTwoProc);
	REGISTERUNITTEST(VNV,Sample2);
	REGISTERSERIALIZER(VNV,doubleToString);
	REGISTERTRANSFORM(VNV,doubleToInt);
	REGISTERCOMM(VNV,mpi);
	REGISTERCOMM(VNV,serial);
	REGISTERREDUCER(VNV,max);
	REGISTERREDUCER(VNV,min);
	REGISTERREDUCER(VNV,prod);
	REGISTERREDUCER(VNV,sqrsum);
	REGISTERREDUCER(VNV,sum);
	REGISTERDATATYPE(VNV,double);
	REGISTERDATATYPE(VNV,float);
	REGISTERDATATYPE(VNV,int);
	REGISTERDATATYPE(VNV,long);
	REGISTERDATATYPE(VNV,longlong);
	REGISTERDATATYPE(VNV,short);
	REGISTERDATATYPE(VNV,string);
	REGISTEROPTIONS(VNV)
	Register_Injection_Point(VNV,configuration,0,"{\"argc\":\"int*\",\"argv\":\"char***\",\"config\":\"nlohmann::json\"}");
	Register_Injection_Point(VNV,initialization,0,"{\"runTests\":\"_Bool\"}");
	REGISTER_FULL_JSON(VNV, getFullRegistrationJson_VNV);
};



