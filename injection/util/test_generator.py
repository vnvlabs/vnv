import argparse
import os

def generateCppFile(name, parameters, includes, sys_includes) :

    s = ""
    s += "#ifndef VV_%s_H \n#define VV_%s_H \n\n" % (name,name);
    
    s += "#include \"injection.h\"\n"
    if includes is not None:
        for include in includes : 
            s += "#include \"%s\"\n" % include[0] 
    if sys_includes is not None:
        for include in sys_includes :
            s += "#include <%s>\n" % include[0]
    s+='\n\n'
    
    s += "class %s : public IVVTest { \npublic:\n\n" % name
    

    s += "\t TestStatus runTest("
    for n,parameter in enumerate(parameters):
        s += "%s* %s" % (parameter[1], parameter[0]) 
        if ( n == len(parameters) - 1 ) :
            s += ") { \n\n\t\t//Enter Test Code Here \n\n\t}\n\n"  
        else :
            s += ","    
    
    s += "\t %s(VVTestConfig config) : IVVTest(config) {\n" %name
    for parameter in parameters :
        s+= "\t\t m_parameters.insert(std::make_pair(\"%s\",\"%s\"));\n" %(parameter[0],parameter[1])
    s += "\t}\n\n"

    s += "\tTestStatus runTest( NTV parameters ) {\n" 
    for parameter in parameters : 
        s += "\t\t%s* %s = carefull_cast<%s>(\"%s\", parameters); \n" %(parameter[1],parameter[0],parameter[1],parameter[0])
    s += "\t\treturn runTest("
    for m,parameter in enumerate(parameters) :
        if ( m > 0 ) : s+="," 
        s+= '%s' % parameter[0]
    s += ");\n"
    s += "\t}\n\n";
    s += "};\n";

    s += "\nextern \"C\" { \n\tIVVTest* %s_maker(VVTestConfig &config) {\n\t\treturn new %s(config);\n\t}\n};\n" % (name,name)
    
    s += "\nclass %s_proxy { \npublic: \n\t%s_proxy(){ \n\t\t test_factory[\"%s\"] = %s_maker;\n\t}\n};\n\n%s_proxy %s_proxy_impl;\n\n" %(name,name,name,name,name,name);
    s += "\n\n#endif"

    return s 

def mdComment(comment): 
    return "[//]: # (%s) \n" % comment 

def generateMarkdownFile(name) :    
    s = mdComment("This is the Markdown file for %s " % name )  
    s +=mdComment("This acts as a postprocessor for the test")
    s +=mdComment("There are quite a few VV specfic extensions supported")
    return s;

parser = argparse.ArgumentParser(description="Generate VV test structure")
parser.add_argument('-n','--name', help='The name of the Test', default='untitled')
parser.add_argument('-p','--parameter', help='The name,type and description of the parameter', nargs='+', action='append')
parser.add_argument('-c','--config', help='name,type,description for a config parameter', nargs='+', action='append')
parser.add_argument('-i','--include', help='local include file', nargs='+', action='append')
parser.add_argument('-si','--sys_include', help='system include file')

args = vars(parser.parse_args())


with open(args["name"] + ".cpp","w") as w:
    w.write(generateCppFile(args["name"] , args["parameter"], args['include'],args['sys_include']) )
with open(os.path.join("..",os.path.join("markdown", args["name"] + ".md")) , "w") as w:
    w.write(generateMarkdownFile(args['name']))
