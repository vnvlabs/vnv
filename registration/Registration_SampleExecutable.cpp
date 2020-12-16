///3990745536566815510
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
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"mpi\",\"package\":\"VNV\"},\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     Whatever we put here will end up in the conclusion.\\n  \",\"InjectionPoints\":{\"Function1\":{\"docs\":\"\\n      Another injection point\\n      =======================\\n\\n      This is another injection point. This one is\\n      part of a global function.\",\"name\":\"Function1\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"},\"End\":{\"docs\":\"\"},\"inner\":{\"docs\":\"\"}}},\"Function1Class1\":{\"docs\":\"\",\"name\":\"Function1Class1\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"},\"End\":{\"docs\":\"\\n       End Function1 Class 1\\n       =====================\\n\\n       The function ended.\\n    \"},\"inner\":{\"docs\":\"\\n Injection point stage Documentation\\n ===================================\\n\\n Injection point stage documentaiton is parsed by the parser, but\\n might not be used depending on the report generator. In either case,\\n it has the same access to the dat aelements of the overall injection\\n points.\\n       \"}}},\"functionTest\":{\"docs\":\"\",\"name\":\"functionTest\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"}}},\"loopTest\":{\"docs\":\"\",\"name\":\"loopTest\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"},\"End\":{\"docs\":\"sdfsdfsdfsdf*\"},\"internal\":{\"docs\":\"Testing stufff\"}}},\"loopTest1\":{\"docs\":\"\",\"name\":\"loopTest1\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"},\"End\":{\"docs\":\"sdfsdfsdfsdf*\"},\"internal\":{\"docs\":\"Testing stufff\"}}},\"loopTest2\":{\"docs\":\"\",\"name\":\"loopTest2\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"},\"End\":{\"docs\":\"sdfsdfsdfsdf*\"},\"internal\":{\"docs\":\"Testing stufff\"}}},\"sdfsdf\":{\"docs\":\"sdfsdfsdfsdf *\",\"name\":\"sdfsdf\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"}}},\"templateFn\":{\"docs\":\"\\n  Template function evaluation\\n  ============================\\n\\n  An example of an injection point inside a template function. Template\\n  functions are interesting because it can be hard to map between injection\\n  points and tests in these cases.\\n   \",\"name\":\"templateFn\",\"packageName\":\"SampleExecutable\",\"parameters\":[null],\"stages\":{\"Begin\":{\"docs\":\"\"}}}},\"Introduction\":\"\\n Sample Executable\\n =================\\n\\n This executable is designed to show a number of different injection points.\\n The first stage is to initialize the library. The text that appears in the\\n comment above the initialize call represents the introduction in the final\\n report.\\n   \",\"LogLevels\":{\"custom\":{\"color\":\"\",\"docs\":\"\\n Create a custom log level with stanadrd coloring.\",\"name\":\"custom\",\"packageName\":\"SampleExecutable\"}},\"Options\":\"\\n Options Documentation\\n =====================\\n\\n This is a quick blurb before the options command that will be used at some\\n point to aid in the development of input files.\\n \",\"SubPackages\":{\"DummyLibOne\":{\"docs\":\"\\n Let the VnV Toolkit know there is a subpackage linked to this executable.\\n \",\"name\":\"DummyLibOne\",\"packageName\":\"SampleExecutable\"}},\"Tests\":{\"sampleTest\":{\"docs\":\"\\n Sample Test As Part of an executable\\n ====================================\\n\\n\\n\\n\\n \",\"parameters\":{\"vals\":\"std::vector<double>\"}}},\"Transforms\":{\"sampleTransform\":{\"docs\":\"\",\"from\":\"double\",\"to\":\"std::vector<double>\"}}}";}

INJECTION_REGISTRATION(SampleExecutable){
	REGISTERTEST(SampleExecutable,sampleTest);
	REGISTERTRANSFORM(SampleExecutable,sampleTransform);
	REGISTERLOGLEVEL(SampleExecutable , custom,);
	REGISTERSUBPACKAGE(SampleExecutable,DummyLibOne);
	REGISTEROPTIONS(SampleExecutable)
	VnV_Declare_Communicator("SampleExecutable","VNV","mpi");
	Register_Injection_Point(SampleExecutable,Function1,0,"null");
	Register_Injection_Point(SampleExecutable,Function1Class1,0,"null");
	Register_Injection_Point(SampleExecutable,functionTest,0,"null");
	Register_Injection_Point(SampleExecutable,loopTest,0,"null");
	Register_Injection_Point(SampleExecutable,loopTest1,0,"null");
	Register_Injection_Point(SampleExecutable,loopTest2,0,"null");
	Register_Injection_Point(SampleExecutable,sdfsdf,0,"null");
	Register_Injection_Point(SampleExecutable,templateFn,0,"null");
	REGISTER_FULL_JSON(SampleExecutable, getFullRegistrationJson_SampleExecutable);
};



