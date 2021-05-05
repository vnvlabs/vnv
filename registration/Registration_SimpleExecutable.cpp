///13969936047787448644
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: SimpleExecutable

#include "VnV.h" 
const char* getFullRegistrationJson_SimpleExecutable(){
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"mpi\",\"package\":\"VNV\"},\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     Whatever we put here will end up in the conclusion.\\n  \",\"Introduction\":\"\\n Simple Executable\\n =================\\n\\n This executable evaluates the function\\n     f = 10x^2 -200 .\\n   \"}";}

INJECTION_REGISTRATION(SimpleExecutable){
	VnV_Declare_Communicator("SimpleExecutable","VNV","mpi");
	REGISTER_FULL_JSON(SimpleExecutable, getFullRegistrationJson_SimpleExecutable);
};



