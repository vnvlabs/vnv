///9300998553531149437
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
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"mpi\",\"package\":\"VNV\"},\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     Whatever we put here will end up in the conclusion.\\n  \",\"InjectionPoints\":{\"Function1\":{\"docs\":\"\\n      Another injection point\\n      =======================\\n\\n      This is another injection point. This one is\\n      part of a global function.\",\"name\":\"Function1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"samplePoints\":\"std::map<double,double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":216,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":136}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":216,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":145}},\"inner\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":1,\"Calling Function Line\":216,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":149}}}},\"Function1Class1\":{\"docs\":\"\",\"name\":\"Function1Class1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"samplePoints\":\"std::vector<double>\",\"samplePoints1\":\"std::vector<double>\",\"samplePoints3\":\"std::vector<double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":79,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":136}},\"End\":{\"docs\":\"\\n       End Function1 Class 1\\n       =====================\\n\\n       The function ended.\\n    \",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":79,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":145}},\"inner\":{\"docs\":\"\\n Injection point stage Documentation\\n ===================================\\n\\n Injection point stage documentaiton is parsed by the parser, but\\n might not be used depending on the report generator. In either case,\\n it has the same access to the dat aelements of the overall injection\\n points.\\n       \",\"info\":{\"Calling Function\":\"function1\",\"Calling Function Column\":3,\"Calling Function Line\":79,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":149}}}},\"NewtonRaphson\":{\"docs\":\"\\n\\n  Newton Raphson Method for finding the root of a function\\n  ========================================================\\n\\n  This function uses the NewtonRaphson method to solve\\n  the function\\n\\n  .. math::\\n\\n     f(x) = x^3 - x^2 + 2 = 0.\\n\\n  where the formula for the Newton Raphson method is:\\n\\n  .. math::\\n\\n     x_{n+1} = x_{n} + \\\\frac{f(x_n)}{f\'(x_n)}.\\n\\n  The solution is x = -1.\\n\\n  The newton raphson method is expected to converge quadratically to\\n  the root. The following chart shows the current value of the best\\n  guess at each iteration of the algorithm.\\n\\n  .. vnv-chart::\\n     :labels: Data.Data[\?Name == \'iter\'].to_string(Value)\\n     :vals: Data.Data[\?Name == \'root\'].Value\\n\\n\\n     {\\n        \\\"type\\\" : \\\"line\\\",\\n        \\\"data\\\" : {\\n           \\\"labels\\\": $$labels$$,\\n           \\\"datasets\\\" : [\\n              {\\n                \\\"label\\\" : \\\"Approximate Root\\\",\\n                \\\"data\\\": $$vals$$,\\n                \\\"fill\\\" : true,\\n                \\\"backgroundColor\\\": \\\"rgb(255, 99, 132)\\\",\\n                \\\"borderColor\\\": \\\"rgb(255, 99, 132)\\\"\\n              }\\n           ]\\n        },\\n        \\\"options\\\" : {\\n           \\\"responsive\\\" : true,\\n           \\\"title\\\" : { \\\"display\\\" : true, \\\"text\\\" : \\\"Convergence of the Newton\\nRaphson Method\\\" }, \\\"scales\\\": { \\\"yAxes\\\": [{ \\\"scaleLabel\\\": { \\\"display\\\": true,\\n                       \\\"labelString\\\": \\\"Current Guess\\\"\\n                   }\\n               }],\\n               \\\"xAxes\\\": [{\\n                   \\\"scaleLabel\\\": {\\n                       \\\"display\\\":true,\\n                       \\\"labelString\\\": \\\"Iteration Number\\\"\\n                   }\\n               }]\\n            }\\n        }\\n     }\",\"name\":\"NewtonRaphson\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"iter\":\"double\",\"rank\":\"int\",\"x\":\"double\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"newtonRaphson\",\"Calling Function Column\":1,\"Calling Function Line\":122,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":136}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"newtonRaphson\",\"Calling Function Column\":1,\"Calling Function Line\":122,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":145}},\"iteration\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"newtonRaphson\",\"Calling Function Column\":1,\"Calling Function Line\":122,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":149}}}},\"functionTest\":{\"docs\":\"\",\"name\":\"functionTest\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"function1\":\"int(int)\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":126}}}},\"loopTest\":{\"docs\":\"\",\"name\":\"loopTest\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":136}},\"End\":{\"docs\":\"sdfsdfsdfsdf*\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":145}},\"internal\":{\"docs\":\"Testing stufff\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":149}}}},\"loopTest1\":{\"docs\":\"\\n\\n  Looped Injection Point with a Callback function\\n  ===============================================\\n\\n  This is a looped injection point with a built in callback function.\\n  Callback functions are usefull as they allow data to be injected directly\\n  into the injection point description (this comment)\\n\\n  In this loop, we iterate across a range [ :vnv:`Data.Data.min`,\\n:vnv:`Data.Data.max`] with a step of :vnv:`Data.Data.count`. At each step,\\nthe INJECTION_LOOP_ITER call is made, representing an internal stage of the\\ninjection point. This is turn calls the injection point call back, which\\nlogs the value of the injection point parameter \\\"aa\\\" (aa is a double set\\nrandomly in each step of the for loop). We plot aa against the step value\\nusing the chart directive.\\n\\n  .. vnv-chart::\\n     :labels: Data.Data[\?Name == \'x\'].to_string(Value)\\n     :vals: Data.Data[\?Name == \'y\'].Value\\n\\n\\n     {\\n        \\\"type\\\" : \\\"line\\\",\\n        \\\"data\\\" : {\\n           \\\"labels\\\": $$labels$$,\\n           \\\"datasets\\\" : [\\n              {\\n               \\\"label\\\" : \\\"A random Number\\\",\\n               \\\"data\\\": $$vals$$,\\n               \\\"fill\\\" : true\\n              }\\n           ]\\n        },\\n        \\\"options\\\" : {\\n           \\\"responsive\\\" : true,\\n           \\\"title\\\" : { \\\"display\\\" : true, \\\"text\\\" : \\\"A sample Graph using the\\nChart directive\\\" }, \\\"yaxis\\\" : { \\\"display\\\" : true, \\\"scaleLabel\\\" : { \\\"display\\\"\\n: true, \\\"labelString\\\": \\\"Value\\\"\\n             }\\n           }\\n        }\\n     }\",\"name\":\"loopTest1\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":136}},\"internal\":{\"docs\":\"Testing stufff\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":149}}}},\"loopTest2\":{\"docs\":\"\",\"name\":\"loopTest2\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":136}},\"End\":{\"docs\":\"sdfsdfsdfsdf*\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":145}},\"internal\":{\"docs\":\"Testing stufff\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":251,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":149}}}},\"sdfsdf\":{\"docs\":\"sdfsdfsdfsdf *\",\"name\":\"sdfsdf\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"ff\":\"double\",\"gg\":\"double\",\"ggg\":\"int\"},{\"ff\":\"int\",\"gg\":\"int\",\"ggg\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"getF\",\"Calling Function Column\":3,\"Calling Function Line\":25,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":126}}}},\"templateFn\":{\"docs\":\"\\n  Template function evaluation\\n  ============================\\n\\n  An example of an injection point inside a template function. Template\\n  functions are interesting because it can be hard to map between injection\\n  points and tests in these cases.\\n   \",\"name\":\"templateFn\",\"packageName\":\"SampleExecutable\",\"parameters\":[{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"double\"},{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"constchar*\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"templateFnc\",\"Calling Function Column\":35,\"Calling Function Line\":63,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/sample.cpp\",\"lineColumn\":5,\"lineNumber\":126}}}}},\"Introduction\":\"\\n Sample Executable\\n =================\\n\\n This executable is designed to show a number of different injection points.\\n The first stage is to initialize the library. The text that appears in the\\n comment above the initialize call represents the introduction in the final\\n report.\\n   \",\"Options\":\"\\n Options Documentation\\n =====================\\n\\n This is a quick blurb before the options command that will be used at some\\n point to aid in the development of input files.\\n \",\"SubPackages\":{\"DummyLibOne\":{\"docs\":\"\\n Let the VnV Toolkit know there is a subpackage linked to this executable.\\n \",\"name\":\"DummyLibOne\",\"packageName\":\"SampleExecutable\"}},\"Tests\":{\"sampleTest\":{\"docs\":\"\\n Sample Test As Part of an executable\\n ====================================\\n\\n\\n\\n\\n \",\"parameters\":{\"vals\":\"std::vector<double>\"}}},\"Transforms\":{\"sampleTransform\":{\"docs\":\"\",\"from\":\"double\",\"to\":\"std::vector<double>\"}}}";}

INJECTION_REGISTRATION(SampleExecutable){
	REGISTERTEST(SampleExecutable,sampleTest);
	REGISTERTRANSFORM(SampleExecutable,sampleTransform);
	REGISTERSUBPACKAGE(SampleExecutable,DummyLibOne);
	REGISTEROPTIONS(SampleExecutable)
	VnV_Declare_Communicator("SampleExecutable","VNV","mpi");
	Register_Injection_Point("SampleExecutable","Function1",0,"{\"samplePoints\":\"std::map<double,double>\"}");
	Register_Injection_Point("SampleExecutable","Function1Class1",0,"{\"samplePoints\":\"std::vector<double>\",\"samplePoints1\":\"std::vector<double>\",\"samplePoints3\":\"std::vector<double>\"}");
	Register_Injection_Point("SampleExecutable","NewtonRaphson",0,"{\"iter\":\"double\",\"rank\":\"int\",\"x\":\"double\"}");
	Register_Injection_Point("SampleExecutable","functionTest",0,"{\"function1\":\"int(int)\"}");
	Register_Injection_Point("SampleExecutable","loopTest",0,"{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}");
	Register_Injection_Point("SampleExecutable","loopTest1",0,"{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}");
	Register_Injection_Point("SampleExecutable","loopTest2",0,"{\"aa\":\"double\",\"count\":\"int\",\"i\":\"int\",\"max\":\"int\",\"min\":\"int\"}");
	Register_Injection_Point("SampleExecutable","sdfsdf",0,"{\"ff\":\"double\",\"gg\":\"double\",\"ggg\":\"int\"}");
	Register_Injection_Point("SampleExecutable","templateFn",0,"{\"x\":\"int\",\"xx\":\"constchar*\",\"y\":\"double\"}");
	REGISTER_FULL_JSON(SampleExecutable, getFullRegistrationJson_SampleExecutable);
};



