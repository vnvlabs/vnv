
import sys

def every_second(count) :
    if (count == 0 ):
        return "#define EVERY_SECOND0(...)\n"
    if ( count == 1) :
        s = "#define EVERY_SECOND1_(second,...), (void*)(&second)  \n"
        s+= "#define EVERY_SECOND1(first,...) EVERY_SECOND1_(__VA_ARGS__)\n"
        return s
    
    s  = "#define EVERY_SECOND" + str(count) + "_(second,...) , (void*)(&second) EVERY_SECOND" + str(count-1) + "(__VA_ARGS__)\n"
    s += "#define EVERY_SECOND" + str(count) +  "(first,...) EVERY_SECOND" + str(count) + "_(__VA_ARGS__)\n"
    return s

def get_every_second_gen(count):
    s = ""
    for i in range(0,count):
        s += every_second(i)
    
    s += "#define COUNT_EVERY_SECOND(" 
    for i in range(1,count):
        s += "_" + str(i) + ",__" + str(i) +","
    s += "num,...) EVERY_SECOND ## num \n"
    
    s += "#define EVERY_SECOND(...) COUNT_EVERY_SECOND(__VA_ARGS__"
    for i in range(count-1,0,-1):
        s+= "," + str(i) + "," + str(i) 
    s+= ",0)(__VA_ARGS__)\n"
    return s

def get_count_every_second(count):

    s = get_every_second_gen(count)
    s += "#define INJECTION_POINT(NAME,...) VV::injectionPoint(#NAME EVERY_SECOND(__VA_ARGS__));\n"
    s += "#define INJECTION_POINT_A(NAME,...) VV::injectionPoint(#NAME, __VA_ARGS__);\n"
    return s

def get_unpacker(count):
    s = "\n\n#define VA_UNPACKER_0(...) \n"
    for i in range(1,count):
        s+= "#define VA_UNPACKER_" + str(i) +"(first,...) , #first VA_UNPACKER_" + str(i-1) + "(__VA_ARGS__)\n"
    
    s += "#define COUNT_UNPACKER(" 
    for i in range(1,count):
        s += "_%d, " % (i) 
    s += "num,...) num VA_UNPACKER_ ## num \n"  
    
    s += "#define VA_UNPACK(...) COUNT_UNPACKER(__VA_ARGS__"
    for i in range(count-1,0,-1):
        s += ",%d" % (i)
    s += ",0)(__VA_ARGS__)\n"
    return s

def get_hard(count):

    s ='''
#define START_INJECTION_POINT 1
#define CP_INJECTION_POINT 2
#define END_INJECTION_POINT 3
#define SINGLE_INJECTION_POINT 4
#define INJECTED_TEST 5
#define INTRODUCTION 6
#define CONCLUSION 7
''' 

    s += get_unpacker(count)
    s += get_every_second_gen(count)

    
    s+= '''
#define INJECTION_POINT(NAME,STAGE) VV_injectionPoint(STAGE,#NAME,__FUNCTION__ EVERY_SECOND( NAME##_VVTest));
#define REGISTER_IP(NAME,STAGE,DESC) static int NAME ## STAGE = VV_registration(#NAME, STAGE , __FILE__,__LINE__,DESC, VA_UNPACK(NAME ## _VVTest) );
#ifdef __cplusplus
    #define EXTERNC extern "C" 
#else
    #define EXTERNC 
#endif

EXTERNC void VV_injectionPoint(int stageVal, const char * id, const char * function, ...); 
EXTERNC int  VV_registration(const char * scope, int stage, const char * filename, int line, const char * desc, int count, ...);
EXTERNC int VV_writeXML(const char *filename);
EXTERNC int VV_init(const char *filename);
EXTERNC int VV_finalize();


'''
    
    return s

if __name__ == "__main__":
    with open("vv-runtime.h", "w") as w:
        w.write(get_hard(int(sys.argv[int(1)])))
