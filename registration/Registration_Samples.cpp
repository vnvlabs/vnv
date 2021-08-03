///18045261148388564671
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: Samples

#include "VnV.h" 
DECLARETEST(Samples,customTest)
DECLARETEST(Samples,customTest2)
DECLARETEST(Samples,customTest3)
DECLARETEST(Samples,customTest4)
DECLAREITERATOR(Samples,sampleIterator)
DECLAREPLUG(Samples,samplePlug)
DECLAREUNITTEST(Samples,test1)
DECLAREUNITTEST(Samples,test2)
DECLAREACTION(Samples,sampleAction)
DECLARETRANSFORM(Samples,AtoB)
DECLARETRANSFORM(Samples,BtoA)
DECLARETRANSFORM(Samples,doubleToInt)
DECLARETRANSFORM(Samples,intToDouble)
DECLARETRANSFORM(Samples,pcToKsp)
DECLAREREDUCER(Samples,custom_reduction)
DECLAREDATATYPE(Samples,customDataType)
DECLAREOPTIONS(Samples)
const char* getFullRegistrationJson_Samples(){
	 return "{\"Actions\":{\"sampleAction\":{\"actions\":{},\"docs\":\"\"}},\"DataTypes\":{\"customDataType\":{\"docs\":\"\"}},\"InjectionPoints\":{\"MY_FIRST_INJECTION_LOOP\":{\"docs\":\"\",\"name\":\"MY_FIRST_INJECTION_LOOP\",\"packageName\":\"Samples\",\"parameters\":[{\"i\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/simple/VnVMacros.cpp\",\"lineColumn\":5,\"lineNumber\":77}},\"End\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/simple/VnVMacros.cpp\",\"lineColumn\":5,\"lineNumber\":86}},\"INSIDE\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/simple/VnVMacros.cpp\",\"lineColumn\":5,\"lineNumber\":90}}}},\"MY_FIRST_INJECTION_POINT_CALLBACK\":{\"docs\":\"\\n My Second Injection Point\\n ------------------------\\n \\n Injection points also let you define a callback routine. To add a callback, you should\\n use the INJECTION_POINT_C call.\\n \\n Inside the callback, you have access to five main parameters. \\n \\n 1. The communication object (comm)\\n 2. The parameter set\\n 3. The output engine, \\n 4. The type of injection point\\n \\n The callback will always be executed each time the injection point call is found. The callback\\n can be used to hard code tests into injection points. \\n \\n As we will outline soon, the callback can also be used to write data that can be used in the \\n template during rendering. For instance, this callback writes the value of argc and uses it \\n when rendering the following.\\n \\n .. note:: The number of arguments passed to the executable was :vnv:`data.argc.value`. \\n \\n We will explain more about writing data and how to reference data in the templates soon. \\n \\n \",\"name\":\"MY_FIRST_INJECTION_POINT_CALLBACK\",\"packageName\":\"Samples\",\"parameters\":[{\"argc\":\"int\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/simple/VnVMacros.cpp\",\"lineColumn\":5,\"lineNumber\":67}}}},\"equal\":{\"docs\":\"\\n  Iterators let you define a region of code that can be iterated over by\\n  an iterator. \\n \\n \\n  Parameter 1: Var -- This is a valid variable name that is available within\\n  the current scope. The Plug macro generates an iterator object that will be \\n  stored in this variable to keep track of the Plug process. \\n \\n  Parameter 2: Package Name\\n  Parameter 3: Communicator\\n  Parameter 4: Unique Plug name \\n  parameter 5: The minimum number of times to run the body when no iterator is supplied. \\n  Parameter 6: The number of input parameters\\n  Parameter 7->7+P5 The input parameters\\n  Parameter 7+P5 -> 25: The output parameters that must be set by the plug. \\n \\n  The body of the Function plug is the default plug. This will be called in \\n  cases where an alternative plug is not provided. \\n \\n  In this case, we have defined an iteration point that sets the output parameter (z) equal\\n  to the input parameter \\\"x\\\".   \\n \\n  In this case we have set \\\"once\\\" to 1. If the users input file does not map an iterator to this\\n  iteration point, the body of the iteration point will be executed one time. \\n \\n    \",\"iterator\":true,\"name\":\"equal\",\"packageName\":\"Samples\",\"parameters\":[{\"x\":\"double\",\"z\":\"double\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":10,\"filename\":\"/home/ben/source/vv/vnv/vv-neams/examples/simple/VnVMacros.cpp\",\"lineColumn\":23,\"lineNumber\":69}}}}},\"Introduction\":\"\",\"Iterators\":{\"sampleIterator\":{\"docs\":\"\\n  Iterators are designed to allow iterative algorithms like UQ/SA/ parameter optimization. \\n  The idea is that the iterator sets the input parameters and analyzes the output parameters. \\n \\n  The process is:\\n \\n  1. VnV calls the iterator asking it to set the input parameters.\\n  2. The iterator sets the input parameters\\n  3. The body of the iteration point is executed using the input parameters.\\n  4. VnV calls the iterator asking it to set the next set of input parameters;\\n  5. The iterator looks at the updated output parameters and decides on the next\\n     set of input parameters. \\n  6 ...... repeat .....\\n  \\n  n: The iterator returns a 0 when it is done. The body of the iteration point will \\n     not be executed again. The output parameters will remain as is.  \\n \\n  This silly little iterator adds one to to input parameter until the output parameter\\n  is greater than 10. In a real iterator, you would want to put some sort of tolerance\\n  such that it cannot run forever. \\n  \\n  Iterators are a special type of test -- You can do anything you can do in a test in a \\n  iterator. The comment above will be rendered in the final report (thats this comment). You \\n  can write data using the engine. \\n \\n \",\"parameters\":{\"x\":\"double\",\"y\":\"double\"}}},\"Options\":\"\\n Packages can define an options schema and callback that allows users to configure\\n the package directly through the input file. \\n \\n See JsonSchema.org for details about json schema. \\n \\n \",\"Plugs\":{\"samplePlug\":{\"docs\":\"\\n  This plug takes a parameter \\\"x\\\", squares it and stores the result in \\n  parameter y. \\n \\n \\n \",\"parameters\":{\"x\":\"double\",\"y\":\"double\"}}},\"Reducers\":{\"custom_reduction\":{\"docs\":\"\"}},\"Tests\":{\"customTest\":{\"docs\":\"\",\"parameters\":{\"x\":\"double\"}},\"customTest2\":{\"docs\":\"\\n My Custom Test 2\\n --------------\\n \\n The sum of x and y was :vnv:`data.sum.value`\\n \",\"parameters\":{\"x\":\"double\",\"y\":\"double\"}},\"customTest3\":{\"docs\":\"\\n Custom test with a runner and a schema\\n --------------------------------------\\n \\n This test uses the INJECTION_TEST_RS macro to define a test with \\n a schema and a runner. It has one test parameter. \\n \\n \",\"parameters\":{\"x\":\"double\"}},\"customTest4\":{\"docs\":\"\\n Handling Injection Point Stages. \\n ==================\\n  \\n Something in here I guess. \\n \",\"parameters\":{\"y\":\"double\"}}},\"Transforms\":{\"AtoB\":{\"docs\":\"\",\"from\":\"B\",\"to\":\"A\"},\"BtoA\":{\"docs\":\"\",\"from\":\"A\",\"to\":\"B\"},\"doubleToInt\":{\"docs\":\"\",\"from\":\"int\",\"to\":\"double\"},\"intToDouble\":{\"docs\":\"\",\"from\":\"double\",\"to\":\"int\"},\"pcToKsp\":{\"docs\":\"\",\"from\":\"KSP\",\"to\":\"PC\"}},\"UnitTests\":{\"test1\":{\"docs\":\"\\n The unit test comment. This is rendered in the final report. \\n \",\"tests\":{\"false\":\"Format is \\\"name\\\", \\\"expected\\\", \\\"got\\\" *\",\"not true\":\"\",\"true\":\"The test assert comment will be rendered in the report *\"}},\"test2\":{\"docs\":\"\\n Parallel unit test\\n \\n \",\"tests\":{\"world Size\":\"The world size should be two. *\"}}}}";}

INJECTION_REGISTRATION(Samples){
	REGISTERTEST(Samples,customTest);
	REGISTERTEST(Samples,customTest2);
	REGISTERTEST(Samples,customTest3);
	REGISTERTEST(Samples,customTest4);
	REGISTERITERATOR(Samples,sampleIterator);
	REGISTERPLUG(Samples,samplePlug);
	REGISTERUNITTEST(Samples,test1);
	REGISTERUNITTEST(Samples,test2);
	REGISTERACTION(Samples,sampleAction);
	REGISTERTRANSFORM(Samples,AtoB);
	REGISTERTRANSFORM(Samples,BtoA);
	REGISTERTRANSFORM(Samples,doubleToInt);
	REGISTERTRANSFORM(Samples,intToDouble);
	REGISTERTRANSFORM(Samples,pcToKsp);
	REGISTERREDUCER(Samples,custom_reduction);
	REGISTERDATATYPE(Samples,customDataType);
	REGISTEROPTIONS(Samples)
	Register_Injection_Point("Samples","MY_FIRST_INJECTION_LOOP","{\"i\":\"int\"}");
	Register_Injection_Point("Samples","MY_FIRST_INJECTION_POINT_CALLBACK","{\"argc\":\"int\"}");
	Register_Injection_Iterator("Samples","equal","{\"x\":\"double\",\"z\":\"double\"}");
	REGISTER_FULL_JSON(Samples, getFullRegistrationJson_Samples);
};



