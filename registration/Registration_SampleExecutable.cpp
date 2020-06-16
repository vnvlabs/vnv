/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

#define PACKAGENAME SampleExecutable
#include "VnV.h" 
DECLARETEST(sampleTest)
DECLARETRANSFORM(sampleTransform)
DECLARESUBPACKAGE(DummyLibOne)
DECLAREOPTIONS
const char* getFullRegistrationJson_SampleExecutable(){
	 return "{\"InjectionPoints\":{\"Function1\":{\"name\":\"Function1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"samplePoints\":\"std::map<double,double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":76,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":82}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":76,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":91}},\"inner\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":76,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":95}}}},\"Function1Class1\":{\"name\":\"Function1Class1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"samplePoints\":\"std::vector<double>\",\"samplePoints1\":\"std::vector<double>\",\"samplePoints3\":\"std::vector<double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":58,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":82}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":58,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":91}},\"inner\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":58,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":95}}}},\"sdfsdf\":{\"name\":\"sdfsdf\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"ff\":\"double\",\"gg\":\"double\",\"ggg\":\"int\"},{\"ff\":\"int\",\"gg\":\"int\",\"ggg\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"getF\",\"Calling Function Column\":3,\"Calling Function Line\":18,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":72}}}},\"templateFn\":{\"name\":\"templateFn\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"double\"},{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"constchar*\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"templateFnc\",\"Calling Function Column\":35,\"Calling Function Line\":51,\"filename\":\"/home/ben/wfr/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":72}}}}},\"LogLevels\":{\"custom\":{\"color\":\"\",\"docs\":\"\\n Create a custom log level with stanadrd coloring.\",\"name\":\"custom\",\"packageName\":\"SampleExecutable\"}},\"Options\":{\"docs\":\"\"},\"SubPackages\":{\"DummyLibOne\":{\"docs\":\"\\n Let the VnV Toolkit know there is a subpackage linked to this executable.\\n \",\"name\":\"DummyLibOne\",\"packageName\":\"SampleExecutable\"}},\"Tests\":{\"sampleTest\":{\"docs\":\"\\n @brief INJECTION_TEST\\n @param vals\\n \",\"parameters\":{\"vals\":\"std::vector<double>\"}}},\"Transforms\":{\"sampleTransform\":{\"docs\":\"\",\"from\":\"double\",\"to\":\"std::vector<double>\"}}}";}

INJECTION_REGISTRATION(){
	REGISTERTEST(sampleTest);
	REGISTERTRANSFORM(sampleTransform);
	REGISTERLOGLEVEL(custom,);
	REGISTERSUBPACKAGE(DummyLibOne);
	REGISTEROPTIONS
	Register_Injection_Point(Function1,"{\"samplePoints\":\"std::map<double,double>\"}");
	Register_Injection_Point(Function1Class1,"{\"samplePoints\":\"std::vector<double>\",\"samplePoints1\":\"std::vector<double>\",\"samplePoints3\":\"std::vector<double>\"}");
	Register_Injection_Point(sdfsdf,"{\"ff\":\"double\",\"gg\":\"double\",\"ggg\":\"int\"}");
	Register_Injection_Point(templateFn,"{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"double\"}");
	REGISTER_FULL_JSON(getFullRegistrationJson_SampleExecutable);
};



