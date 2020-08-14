///10388483141274751923
/// This file was automatically generated using the VnV-Matcher executable. 
/// The matcher allows for automatic registration of all VnV plugins and injection 
/// points. Building the matcher requires Clang. If Clang is not available on this machine,
/// Registration code should be written manually. 
/// 

//PACKAGENAME: Papi

#include "VnV.h" 
DECLARETEST(Papi,flops)
DECLARETEST(Papi,hardware_info)
const char* getFullRegistrationJson_Papi(){
	 return "{\"Tests\":{\"flops\":{\"docs\":\"\\n Recorded Floating point operations and cycles.\\n ==============================================\\n\\n The figure below shows the number of floating point operations recorded\\n using PAPI throughout the duration of this injection point.\\n\\n\\n .. vnv-chart::\\n    :labels: Data[\?Name == \'stage\'].Value\\n    :flops: Data[\?Name == \'fpins\'].Value\\n    :cycs: Data[\?Name == \'cycles\'].Value\\n\\n    {\\n       \\\"type\\\" : \\\"line\\\",\\n       \\\"data\\\" : {\\n          \\\"labels\\\" : $$labels$$,\\n          \\\"datasets\\\" : [\\n             {\\n               \\\"label\\\": \\\"Recorded Floating Point Operations\\\",\\n               \\\"borderColor\\\": \\\"rgb(0, 255, 0)\\\",\\n               \\\"data\\\": $$flops$$,\\n               \\\"yAxisID\\\" : \\\"A\\\"\\n             },\\n             {\\n               \\\"label\\\": \\\"Recorded Cycles\\\",\\n               \\\"borderColor\\\": \\\"rgb(255, 0, 0)\\\",\\n               \\\"data\\\": $$cycs$$,\\n               \\\"yAxisID\\\" : \\\"B\\\"\\n             }\\n          ]\\n       },\\n       \\\"options\\\" : {\\n           \\\"scales\\\" : {\\n             \\\"yAxes\\\" : [\\n                { \\\"id\\\" : \\\"A\\\" , \\\"position\\\" : \\\"left\\\", \\\"type\\\" : \\\"linear\\\" },\\n                { \\\"id\\\" : \\\"B\\\", \\\"position\\\" : \\\"right\\\", \\\"type\\\" : \\\"linear\\\" }\\n             ]\\n\\n           }\\n       }\\n    }\\n\\n .. note::\\n    Counters will include any cost associated with injection point tests in child nodes. Users should use caution when using nested profiling with this toolkit.\\n\\n \",\"parameters\":{}},\"hardware_info\":{\"docs\":\"\\n COMPUTER HARDWARE AND MEMORY INFORMATION\\n ========================================\\n\\n The compute hardware information is:\\n\\n .. vnv-jchart::\\n    :main: Data.hardware.Value\\n\\n    $$main$$\\n\\n \",\"parameters\":{}}}}";}

INJECTION_REGISTRATION(Papi){
	REGISTERTEST(Papi,flops);
	REGISTERTEST(Papi,hardware_info);
	REGISTER_FULL_JSON(Papi, getFullRegistrationJson_Papi);
};



