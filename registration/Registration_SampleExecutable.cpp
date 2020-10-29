///12624880266350685799
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: SampleExecutable

#include "VnV.h" 
DECLARETEST(SampleExecutable,sampleTest)
DECLARETRANSFORM(SampleExecutable,sampleTransform)
DECLARESUBPACKAGE(DummyLibOne)
DECLAREOPTIONS(SampleExecutable)
const char* getFullRegistrationJson_SampleExecutable(){
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"mpi\",\"package\":\"VNV\"},\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     Whatever we put here will end up in the conclusion.\\n  \",\"InjectionPoints\":{\"Function1\":{\"docs\":\"\\n      Another injection point\\n      =======================\\n\\n      This is another injection point. This one is\\n      part of a global function.\",\"name\":\"Function1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"samplePoints\":\"std::map<double,double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":117,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":89}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":117,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":98}},\"inner\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":117,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":102}}}},\"Function1Class1\":{\"docs\":\"\",\"name\":\"Function1Class1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"samplePoints\":\"std::vector<double>\",\"samplePoints1\":\"std::vector<double>\",\"samplePoints3\":\"std::vector<double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":79,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":89}},\"End\":{\"docs\":\"\\n       End Function1 Class 1\\n       =====================\\n\\n       The function ended.\\n    \",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":79,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":98}},\"inner\":{\"docs\":\"\\n Injection point stage Documentation\\n ===================================\\n\\n Injection point stage documentaiton is parsed by the parser, but\\n might not be used depending on the report generator. In either case,\\n it has the same access to the dat aelements of the overall injection\\n points.\\n       \",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":79,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":102}}}},\"functionTest\":{\"docs\":\"\",\"name\":\"functionTest\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"function1\":\"int(int)\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":79}}}},\"loopTest\":{\"docs\":\"\",\"name\":\"loopTest\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":89}},\"End\":{\"docs\":\"sdfsdfsdfsdf*\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":98}},\"internal\":{\"docs\":\"Testing stufff\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":102}}}},\"loopTest1\":{\"docs\":\"\",\"name\":\"loopTest1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":89}},\"End\":{\"docs\":\"sdfsdfsdfsdf*\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":98}},\"internal\":{\"docs\":\"Testing stufff\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":102}}}},\"loopTest2\":{\"docs\":\"\",\"name\":\"loopTest2\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":89}},\"End\":{\"docs\":\"sdfsdfsdfsdf*\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":98}},\"internal\":{\"docs\":\"Testing stufff\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":158,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":102}}}},\"sdfsdf\":{\"docs\":\"sdfsdfsdfsdf *\",\"name\":\"sdfsdf\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"ff\":\"double\",\"gg\":\"double\",\"ggg\":\"int\"},{\"ff\":\"int\",\"gg\":\"int\",\"ggg\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"getF\",\"Calling Function Column\":3,\"Calling Function Line\":25,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":79}}}},\"templateFn\":{\"docs\":\"\\n  Template function evaluation\\n  ============================\\n\\n  An example of an injection point inside a template function. Template\\n  functions are interesting because it can be hard to map between injection\\n  points and tests in these cases.\\n   \",\"name\":\"templateFn\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"double\"},{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"constchar*\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"templateFnc\",\"Calling Function Column\":35,\"Calling Function Line\":64,\"filename\":\"/home/ben/wfr/vv-neams/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":79}}}}},\"Introduction\":\"\\n Sample Executable\\n =================\\n\\n This executable is designed to show a number of different injection points.\\n The first stage is to initialize the library. The text that appears in the\\n comment above the initialize call represents the introduction in the final\\n report.\\n   \",\"LogLevels\":{\"custom\":{\"color\":\"\",\"docs\":\"\\n Create a custom log level with stanadrd coloring.\",\"name\":\"custom\",\"packageName\":\"SampleExecutable\"}},\"Options\":\"\\n Options Documentation\\n =====================\\n\\n This is a quick blurb before the options command that will be used at some\\n point to aid in the development of input files.\\n \",\"SubPackages\":{\"DummyLibOne\":{\"docs\":\"\\n Let the VnV Toolkit know there is a subpackage linked to this executable.\\n \",\"name\":\"DummyLibOne\",\"packageName\":\"SampleExecutable\"}},\"Tests\":{\"sampleTest\":{\"docs\":\"\\n Sample Test As Part of an executable\\n ====================================\\n\\n\\n\\n\\n \",\"parameters\":{\"vals\":\"std::vector<double>\"}}},\"Transforms\":{\"sampleTransform\":{\"docs\":\"\",\"from\":\"double\",\"to\":\"std::vector<double>\"}}}";}

INJECTION_REGISTRATION(SampleExecutable){
	REGISTERTEST(SampleExecutable,sampleTest);
	REGISTERTRANSFORM(SampleExecutable,sampleTransform);
	REGISTERLOGLEVEL(SampleExecutable , custom,);
	REGISTERSUBPACKAGE(SampleExecutable,DummyLibOne);
	REGISTEROPTIONS(SampleExecutable)
	VnV_Declare_Communicator("SampleExecutable","VNV","mpi");
	Register_Injection_Point(SampleExecutable,Function1,"{\"samplePoints\":\"std::map<double,double>\"}");
	Register_Injection_Point(SampleExecutable,Function1Class1,"{\"samplePoints\":\"std::vector<double>\",\"samplePoints1\":\"std::vector<double>\",\"samplePoints3\":\"std::vector<double>\"}");
	Register_Injection_Point(SampleExecutable,functionTest,"{\"function1\":\"int(int)\"}");
	Register_Injection_Point(SampleExecutable,loopTest,"{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}");
	Register_Injection_Point(SampleExecutable,loopTest1,"{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}");
	Register_Injection_Point(SampleExecutable,loopTest2,"{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}");
	Register_Injection_Point(SampleExecutable,sdfsdf,"{\"ff\":\"double\",\"gg\":\"double\",\"ggg\":\"int\"}");
	Register_Injection_Point(SampleExecutable,templateFn,"{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"double\"}");
	REGISTER_FULL_JSON(SampleExecutable, getFullRegistrationJson_SampleExecutable);
};



