

def every_second(count) :
    if (count == 0 ):
        return "#define EVERY_SECOND0(...)\n"
    if ( count == 1) :
        s = "#define EVERY_SECOND1_(second,...), &second  \n"
        s+= "#define EVERY_SECOND1(first,...) EVERY_SECOND1_(__VA_ARGS__)\n"
        return s
    
    s  = "#define EVERY_SECOND" + str(count) + "_(second,...) , &second EVERY_SECOND" + str(count-1) + "(__VA_ARGS__)\n"
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
    s += "num,...) VA_UNPACKER_ ## num \n "  
    
    s += "#define VA_UNPACK(...) COUNT_UNPACKER(__VA_ARGS__"
    for i in range(count-1,0,-1):
        s += ",%d" % (i)
    s += ",0)(__VA_ARGS__)\n"
    return s

def get_hard(count):

    s = get_unpacker(count)
    s += get_every_second_gen(count)

    s+= "#define INJECTION_POINT_START(NAME,DESC) VV::injectionPoint(START,#NAME,__FUNCTION__,DESC EVERY_SECOND( NAME##_VVTest));\n"
    s+= "#define INJECTION_POINT_END(NAME,DESC) VV::injectionPoint(END,#NAME,__FUNCTION__, DESC EVERY_SECOND( NAME##_VVTest));\n"
    s+= "#define INJECTION_POINT_CP(NAME,DESC) VV::injectionPoint(CP,#NAME,__FUNCTION__, DESC EVERY_SECOND( NAME##_VVTest));\n"
    s+= "#define INJECTION_POINT(NAME,DESC) VV::injectionPoint(SINGLE,#NAME,__FUNCTION__, DESC EVERY_SECOND( NAME##_VVTest));\n"
    
    s+= "#define REGISTER_INJECTION_POINT(NAME,DESC)  InjectionPointRegistrar NAME(#NAME,__FILE__,__LINE__,DESC VA_UNPACK(NAME ## _VVTest) );\n"
    
    return s

if __name__ == "__main__":
    with open("vv_macros.h", "w") as w:
        w.write(get_hard(25))
