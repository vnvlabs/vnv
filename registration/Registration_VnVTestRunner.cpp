///13054456563472598110
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: VnVTestRunner

#include "VnV.h" 
DECLAREUNITTEST(VnVTestRunner,AnotherDemo)
DECLAREUNITTEST(VnVTestRunner,BFSTester)
DECLAREUNITTEST(VnVTestRunner,Communication)
DECLAREUNITTEST(VnVTestRunner,Demo)
DECLAREUNITTEST(VnVTestRunner,bitvector)
DECLAREUNITTEST(VnVTestRunner,router)
const char* getFullRegistrationJson_VnVTestRunner(){
	 return "{\"Conclusion\":\"\",\"Introduction\":\"\",\"UnitTests\":{\"AnotherDemo\":{\"docs\":\"\",\"tests\":{\"size==2\":\"\"}},\"BFSTester\":{\"docs\":\"\\n Breath First Search function\\n The breadth first search function is used in the Transformation API to automatically\\n transform variables between injection point parmeters and test parameters.\\n \",\"tests\":{\"Empty Set When From == To \":\"\\n This test ensures the breadth first search algorithm returns an\\n empty set when the from node equals the to node. The test is\\n run using\\n\\n .. code-block:: cpp\\n    :linenos:\\n\\n    VnV::bfs(graph,\\\"from\\\",\\\"from\\\").size() == 0\\n   \",\"FromNotInMap\":\"\",\"Loops\":\"\",\"One Level\":\"\",\"To not in set\":\"\",\"Two Level\":\"\"}},\"Communication\":{\"docs\":\"\",\"tests\":{\"All Reduce\":\"\",\"Comm compare with different comm\":\"\",\"Comm compare with duplicate of self\":\"\",\"Comm compare with self\":\"\",\"Gather\":\"\",\"ISend -> Probe -> Recv \":\"\",\"Probe Source\":\"\",\"Probe Tag\":\"\",\"Send And Recv\":\"\",\"Send IRecv and Wait\":\"\",\"bcast from 3, read at 0\":\"\",\"comm contains\":\"\",\"comm not contained in \":\"\",\"reduce LAND\":\"\",\"reduce LOR\":\"\",\"reduce SUM\":\"\",\"size\":\"\"}},\"Demo\":{\"docs\":\"\",\"tests\":{\"x == 10\":\"\",\"x == y\":\"\",\"x+y==20\":\"\",\"x-y==20\":\"\",\"y == 10\":\"\"}},\"bitvector\":{\"docs\":\"\\n Unit tests for the sparse bit vector code.\\n \",\"tests\":{}},\"router\":{\"docs\":\"\\n Unit Tests for the Router Class of the Parallel Output Engine.\\n\\n .. vnv-unit-test-results::\\n\\n\\n \",\"tests\":{}}}}";}

INJECTION_REGISTRATION(VnVTestRunner){
	REGISTERUNITTEST(VnVTestRunner,AnotherDemo);
	REGISTERUNITTEST(VnVTestRunner,BFSTester);
	REGISTERUNITTEST(VnVTestRunner,Communication);
	REGISTERUNITTEST(VnVTestRunner,Demo);
	REGISTERUNITTEST(VnVTestRunner,bitvector);
	REGISTERUNITTEST(VnVTestRunner,router);
	REGISTER_FULL_JSON(VnVTestRunner, getFullRegistrationJson_VnVTestRunner);
};



