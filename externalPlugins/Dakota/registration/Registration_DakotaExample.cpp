///12858948136534122287
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: DakotaExample

#include "VnV.h" 
const char* getFullRegistrationJson_DakotaExample(){
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"serial\",\"package\":\"VNV\"},\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     That concludes the Dakota example -- I hope you enjoyed the show.\\n  \",\"InjectionPoints\":{\"dakotaLoop\":{\"docs\":\"\",\"iterator\":true,\"name\":\"dakotaLoop\",\"packageName\":\"DakotaExample\",\"parameters\":[{\"f\":\"double\",\"x\":\"double\",\"y\":\"double\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":8,\"filename\":\"/home/ben/work/vnv/vv-neams/externalPlugins/Dakota/examples/sample.cpp\",\"lineColumn\":51,\"lineNumber\":148}}}}},\"Introduction\":\"\\n Dakota Example\\n =================\\n\\n This executable deomstrates using the dakota tests at a VnV Iteration\\n point.\\n   \"}";}

INJECTION_REGISTRATION(DakotaExample){
	VnV_Declare_Communicator("DakotaExample","VNV","serial");
	Register_Injection_Point(DakotaExample,dakotaLoop,1,"{\"f\":\"double\",\"x\":\"double\",\"y\":\"double\"}");
	REGISTER_FULL_JSON(DakotaExample, getFullRegistrationJson_DakotaExample);
};



