///313380492635950935
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: Distributed

#include "VnV.h" 
DECLARETEST(Distributed,put_matrix)
DECLARETEST(Distributed,put_rank)
DECLARETEST(Distributed,put_reduce)
DECLARETEST(Distributed,write_vec)
DECLAREREDUCER(Distributed,custom_reduction)
const char* getFullRegistrationJson_Distributed(){
	 return "{\"Communicator\":{\"docs\":\"\",\"name\":\"mpi\",\"package\":\"VNV\"},\"Conclusion\":\"\\n     Conclusion.\\n     ===========\\n\\n     Whatever we put here will end up in the conclusion.\\n  \",\"InjectionPoints\":{\"distribute\":{\"docs\":\"\",\"name\":\"distribute\",\"packageName\":\"Distributed\",\"parameters\":[{\"local_mat\":\"std::vector<std::vector<double>>\",\"local_vec\":\"std::vector<double>\"}],\"stages\":{\"Begin\":{\"docs\":\"\",\"info\":{\"Calling Function\":\"main\",\"Calling Function Column\":1,\"Calling Function Line\":156,\"filename\":\"/home/ben/source/vv-neams/examples/dummy/executables/distributed.cpp\",\"lineColumn\":5,\"lineNumber\":126}}}}},\"Introduction\":\"\\n Distributed Data Example\\n ------------------------\\n\\n This executable demonstrated the distributed data capabilities of the\\n IOutputEngine interface. The example provides a single injection point with\\n a vector of doubles as the only available input parameter.\\n\\n In this example, we provide several VnV tests demoing the distributed data\\n support. These test expect a vector of doubles as input and write data to\\n using the various distributed data functions.\\n\\n The example expects a file called \\\"vv-dist-data.json\\\"  to be available in\\n the directory it is run from.\\n\\n The executable expects two command line arguements. The first represents\\n the local size of the vector on each process. The second parameter\\n represents the size of the matrix on each local process for the matrix\\n examples.\\n\\n For example mpirun -n p <exe> 10 3 will create a vector of length p*size\\n and a matrix of size [3*p,3*p]\\n\\n\\n .. hint::\\n    This comment will show up in the final report as the introduction. It\\n supports restructured text markup.\\n   \",\"Reducers\":{\"custom_reduction\":{\"docs\":\"\"}},\"Tests\":{\"put_matrix\":{\"docs\":\"\\n Put_Matrix Test examples.\\n -----------------------\\n\\n This test demos the Put_Matrix methods. These methods write vector\\n data from ONE process with rank = r.\\n\\n TODO/Questions\\n Do we want a standard VnV Way to define domain decomposition for\\n a matrix. This test only works because I know the matrix is row-wise\\n More information is needed to generate more data.\\n\\n \",\"parameters\":{\"x\":\"std::vector<std::vector<double>>\"}},\"put_rank\":{\"docs\":\"\\n Put_Rank Test examples.\\n -----------------------\\n\\n This test demos the Put_Rank methods. These methods write vector\\n data from ONE process with rank = r.\\n\\n template<typename T>\\n auto& x() { return getReference<std::vector<double> x>(\\\"std::vector<double>\\n x\\\");\\n \",\"parameters\":{\"x\":\"std::vector<double>\"}},\"put_reduce\":{\"docs\":\"\\n Reduction Demonstration\\n -----------------------\\n\\n Demonstrates the reduction interface. These API functions allow\\n a vector to be reduced across the communicator.\\n \",\"parameters\":{\"x\":\"std::vector<double>\"}},\"write_vec\":{\"docs\":\"\",\"parameters\":{\"x\":\"std::vector<double>\"}}}}";}

INJECTION_REGISTRATION(Distributed){
	REGISTERTEST(Distributed,put_matrix);
	REGISTERTEST(Distributed,put_rank);
	REGISTERTEST(Distributed,put_reduce);
	REGISTERTEST(Distributed,write_vec);
	REGISTERREDUCER(Distributed,custom_reduction);
	VnV_Declare_Communicator("Distributed","VNV","mpi");
	Register_Injection_Point("Distributed","distribute",0,"{\"local_mat\":\"std::vector<std::vector<double>>\",\"local_vec\":\"std::vector<double>\"}");
	REGISTER_FULL_JSON(Distributed, getFullRegistrationJson_Distributed);
};



