
#include "injection.h"

using namespace vv;

int InjectionPointBaseFactory::StringSplit(const std::string &s,
                    const char *delim,
                    std::vector< std::string > &result )
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    result.clear();
    while ( std::getline( ss,item,delim[0]) )
    {
        if (!item.empty())
            result.push_back( item );
    }
    return 1;
}



VVTestLibrary::VVTestLibrary(std::string path) {
    libraryPath = path;
}

void VVTestLibrary::openLibrary() {
    if (status == CLOSED) {
        dlib_entry = dlopen(libraryPath.c_str(), RTLD_NOW);
        if ( dlib_entry == NULL ) {
            std::cerr << dlerror() << std::endl;
            status = ERROR;
        } else {
            status = OPEN;
        }
    }
}

void VVTestLibrary::closeLibrary() {
    if (status == OPEN) {
        dlclose(dlib_entry);
    }
}

VVTestLibrary::~VVTestLibrary() {
}


InjectionPoint::InjectionPoint(std::string scope, std::string markdown ) : m_markdown(markdown), m_scope(scope) {
}

InjectionPointStage::InjectionPointStage(std::string markdown) : m_empty(true) {}

InjectionPointStage::InjectionPointStage(std::string filename, int lineNumber, std::string desc, plist &params ) :
    m_filename(filename),
    m_lineNumber(lineNumber) ,
    m_desc(desc),
    m_params(params)
{}

void InjectionPointStage::setFromEmpty(std::string filename, int lineNumber, std::string desc, plist &params ) {

    m_filename = filename;
    m_lineNumber = lineNumber;
    m_desc = desc;
    m_empty = false;
    m_params = params;
}

std::string InjectionPoint::getScope() const {
    return m_scope;
}

std::string InjectionPointStage::getFilename() const {
    return m_filename;
}

int InjectionPointStage::getLineNumber() const {
    return m_lineNumber;
}

plist InjectionPointStage::getParams() const {
    return m_params;
}

std::string InjectionPointStage::getDescription() const {
    return m_desc;
}

void InjectionPoint::addTest(VVTestConfig config) {

    auto it = InjectionPointBaseFactory::test_factory.find(config.testName);
    if ( it != InjectionPointBaseFactory::test_factory.end() ) {
        m_tests.push_back(it->second.first(config));
    }
}

bool InjectionPointStage::isEmpty() {
    return m_empty;
}

InjectionPoint InjectionPointBaseFactory::getDescription(std::string const&scope) {

    ip_map::iterator it = getMap()->find(scope);
    if ( it == getMap()->end()) {
        throw "Injection Point Not Found";
    }
    return it->second;
}

InjectionPoint::InjectionPoint(std::string scope, int stage, std::string filename, int lineNumber, std::string desc, plist &params ) :
    m_scope(scope)
{
    stages.insert(std::make_pair(stage, InjectionPointStage(filename,lineNumber, desc, params)));
}

void InjectionPoint::addStage(int stageIndex, std::string filename, int lineNumber, std::string desc, plist &params) {

    auto s = stages.find(stageIndex);
    if ( s == stages.end() ) {
        stages.insert(std::make_pair(stageIndex, InjectionPointStage(filename,lineNumber, desc, params)));
    } else if ( s->second.isEmpty() ) {
        s->second.setFromEmpty(filename, lineNumber, desc, params);
    }
}

void VV_injectionPoint(int stageVal, const char * id, const char * function, ...) {

    va_list argp;
    va_start(argp, function);
    InjectionPointBaseFactory::injectionPoint(stageVal, id, function, argp);
    va_end(argp);
}

void InjectionPointBaseFactory::printAllEngines() {

    InjectionPointBaseFactory::getAll();
    
    std::cout << " All Available Engines " << std::endl;
    for (auto it : registeredEngines ) {
      std::cout << "\t" << it.first << std::endl; 
    }
    std::cout << "End registered Engines" << std::endl; 
    
    std::cout << " All Available Tests -- Note: Tests are registered through the input file.  " << std::endl;
    for (auto it : test_factory ) {
      std::cout << "\t" << it.first << std::endl; 
    }
    std::cout << "End registered Tests" << std::endl; 

    std::cout << " All Available Transformater -- Note: Transformers are registered through the input file.  " << std::endl;
    for (auto it : trans_factory ) {
      std::cout << "\t" << it.first << std::endl; 
    }
    std::cout << "End registered Transformer" << std::endl; 

}


int VV_init(const char * filename) {
  InjectionPointBaseFactory::VVInit(filename);
  return 1;
}

int VV_finalize() {
  InjectionPointBaseFactory::VVFinalize();
  return 1;
}
int VV_writeXML(const char * filename) {
  InjectionPointBaseFactory::writeXMLFile(filename);
  return 1;
}

int VV_registration(const char * scope, int stage, const char * filename, int line, const char * desc, int count, ...) {
    va_list argp;
    va_start(argp,count);
    InjectionPointRegistrar r(scope, stage, filename, line, desc, count, argp);
    va_end(argp);
    return 0;
}

void InjectionPointBaseFactory::injectionPoint(int injectionIndex, std::string scope, std::string function, ...) {
    va_list argp;
    va_start(argp, function);
    injectionPoint(injectionIndex, scope, function, argp);
}

void InjectionPointBaseFactory::injectionPoint(int injectionIndex, std::string scope, std::string function, va_list argp) {

    if ( runTests ) {
        auto ipd =  InjectionPointBaseFactory::getDescription(scope);
        ipd.runTests(injectionIndex, argp );
    }

}

void InjectionPoint::unpack_parameters(int stageValue, NTV &ntv, va_list argp) {

    auto s = stages.find(stageValue);
    if ( s != stages.end() ) {

        int numParams = s->second.m_params.size(); // number of params we are expecting.

        for ( int x = 0; x < numParams; x++ )
        {
            ntv.insert(std::make_pair(s->second.m_params[x].first, std::make_pair(s->second.m_params[x].second, va_arg(argp,void*))));
        }
        va_end ( argp );                  // Cleans up the list
    }
}

void InjectionPoint::runTests(int stageValue ,va_list argp ) {

    auto s = stages.find(stageValue);
    if ( s != stages.end() ) {
        // Write BeginStep Injection point scope
        VVOutputEngineManager *wrapper = InjectionPointBaseFactory::manager;
        NTV ntv;
        unpack_parameters(stageValue, ntv, argp);

        // Call the method to write this injection point to file.
        wrapper->startInjectionPoint(getScope(),stageValue, m_markdown);
        for ( auto it : m_tests ) {
            it->_runTest(wrapper->getOutputEngine(), stageValue,ntv);
        }
        wrapper->endInjectionPoint(getScope(),stageValue, m_markdown);
    }
}

ip_map* InjectionPointBaseFactory::map = NULL;

std::vector<void*> InjectionPointBaseFactory::testLibraries;
IntroStruct InjectionPointBaseFactory::intro;
OutroStruct InjectionPointBaseFactory::outro;
std::set<std::string> InjectionPointBaseFactory::registeredTests;


void InjectionPointBaseFactory::getAll() {

    ip_map::iterator it = getMap()->begin();
    std::cout << "Printing all registered Injection points: " << std::endl;
    while ( it != getMap()->end() ) {
        std::cout << it->second.getScope() << std::endl;
        it++;
    }
    std::cout << "Finished printing available injection points" << std::endl;

}

ip_map* InjectionPointBaseFactory::getMap() {
    if (!map) {
        map = new ip_map;
    }
    return map;
}

void InjectionPointBaseFactory::populateScope(scope_p scope, InjectionPoint &ipd) {

    injectionPoint_p ip = scope->add_node_injectionPoint();

    for ( auto it : ipd.stages ) {

        injectionPointStage_p ips = ip->add_node_injectionPointStage();
        ips->set_desc(it.second.getDescription());
        plist params = it.second.getParams();
        std::ostringstream oss;
        int count = 0;
        oss << "InjectionPoint(";
        for ( auto ss : params ) {
            if (count == 0 )  count ++;
            else oss<< ", " ;
            oss << ss.first << " " << ss.second ;
        }
        oss << ");";
        ips->set_parameters(oss.str().c_str()  );
        ips->set_attr_stageId(it.first);
    }
    ip->set_attr_name(ipd.getScope());
    ip->set_attr_markdown("TODO");
}


void InjectionPointBaseFactory::populateDocument(vv::Document *doc) {
    exe_p mainExe = doc->element_exe();
    mainExe->set_attr_name("sdfsdfs");
    mainExe->set_path("sdfsdf");

    mainExe->mark_present_intro();
    intro_p intro = mainExe->element_intro();
    intro->set_markdown("default.md");


    mainExe->mark_present_outro();
    outro_p outro = mainExe->element_outro();
    outro->set_markdown("default.md");


    testLibrary_p tl = mainExe->add_node_testLibrary();
    tl->add_path_string("injection/default-test-lib/libdefault-test.so");

    scope_p mainScope = mainExe->add_node_scope();
    mainScope->set_attr_name("global");

    ip_map *map = InjectionPointBaseFactory::getMap();
    for ( auto ipd : *map ) {

        std::vector<std::string> split_scope;
        std::string scope = ipd.second.getScope();
        scope = "global_" + scope;
        InjectionPointBaseFactory::StringSplit(scope,"_",split_scope);

        scope_p current_scope = mainScope; // start at the root.
        for ( auto it = split_scope.begin(); it != split_scope.end(); it++ ) {
            if ( current_scope->get_attr_name_string() == *it ) {}
            else if ( it + 1 == split_scope.end() ) {
                // This is the final line in the scope, so it must be a injection point.
                populateScope(current_scope,ipd.second);
            }
            else {
                for ( auto cscope : current_scope->elements_scope() ) {
                    if ( cscope->get_attr_name_string() == *it ) {
                        current_scope = cscope;
                        break;
                    }
                }

                current_scope = current_scope->add_node_scope();
                current_scope->set_attr_name(*it);
            }
        }
    }
}

void InjectionPointBaseFactory::DeclareTest(std::string testName) {
    auto it = InjectionPointBaseFactory::test_factory.find(testName);
    auto itt = registeredTests.find(testName);

    if ( itt == registeredTests.end() && it != InjectionPointBaseFactory::test_factory.end() ) {
        it->second.second(InjectionPointBaseFactory::manager->getOutputEngine());
        registeredTests.insert(testName);
    }

}

void InjectionPointBaseFactory::addTest(test_p t, InjectionPoint &ipd) {
    std::string testName = t->get_attr_name_string();
    std::string markdown = t->get_attr_markdown_string();
    
    DeclareTest(testName);
    
    std::cout << " Adding the TESTS " << testName <<std::endl;

    VVTestConfig c;
    c.testName = t->get_attr_name_string();
    c.markdown = t->get_attr_markdown_string();
    for ( auto p : t->elements_testStage() ) {
        VVTestStageConfig config;
        config.testStageId = p->get_attr_testId();
        config.injectionPointStageId = p->get_attr_ipId();
        for ( auto param : p->elements_parameter() ) {
            config.addTransform(param->get_attr_to_string(), param->get_attr_from_string(), param->get_attr_trans_string());
        }
        for (auto conf : t->elements_config() ) {
            c.config_params.insert( std::make_pair( conf->get_attr_name_string(),conf->get_attr_value_string()));
        }
        c.addTestStage(config);
    }
  
    std::cout << " Adding the TESTS " << testName <<std::endl;
    ipd.addTest(c);
}

void InjectionPointBaseFactory::addInjectionPoint(std::string scope, injectionPoint_p ip) {

    std::string name = ip->get_attr_name_string();
    std::string markdown = ip->get_attr_markdown_string();

    ip_map *map = getMap();

    auto ipd_iter = map->find(name);
    if ( ipd_iter == map->end() ) {
        // This injection point is not yet registered -- maybe it is lazy loaded -- so add in
        // a empty injection point that can be overriden later .
        auto in = map->insert( std::make_pair(name, InjectionPoint(scope, markdown) ) );
        for (auto test : ip->elements_test() ) {
            addTest(test, in.first->second);
        }

    } else {
        for (auto test : ip->elements_test() ) {
            addTest(test, ipd_iter->second);
        }
        ipd_iter->second.m_markdown = markdown;
    }
}

void InjectionPointBaseFactory::addTestLibrary(testLibrary_p lib) {
    int i = 0;
    for ( auto it : lib->elements_path() ) {
        try {
            void * dllib = dlopen(lib->get_path_string(i++).c_str(), RTLD_NOW);
            if ( dllib == NULL )
                std::cerr << dlerror();
            else {
                testLibraries.push_back( dllib );
            }
        } catch(...) {
            std::cout << "Library not found at path " << lib->get_path_string(i-1) << "\n";
        }
    }
}

void InjectionPointBaseFactory::addScope(scope_p scope, std::string level) {

    std::string scopeName = scope->get_attr_name_string() ;
    for ( auto ip : scope->elements_injectionPoint() ) {
        addInjectionPoint(scopeName, ip);
    }
    for ( auto cscope : scope->elements_scope() ) {
        addScope(cscope,scopeName);
    }
}

void InjectionPointBaseFactory::parseXMLFile(vv::Document *doc ) {


    exe_p exe = doc->element_exe();

    for ( auto lib : exe->elements_testLibrary() ) {
        addTestLibrary(lib);
    }
    for ( auto scope : exe->elements_scope() ) {
        addScope(scope);
    }

    intro_p vvintro = exe->element_intro() ;
    if (vvintro != NULL ) {
        intro.intoMarkdown = vvintro->get_markdown_string();
    } else {
        intro.intoMarkdown = "";
    }
    outro_p vvoutro = exe->element_outro() ;
    if (vvoutro != NULL ) {
        outro.outroMarkdown = vvoutro->get_markdown_string();
    } else {
        outro.outroMarkdown = "";
    }

}


void InjectionPointBaseFactory::parseXMLFile(std::string filename) {

    // Parse the xml file.
    //


    try {
        vv::Document * doc = new vv::Document(false);
        doc->prettyPrint(true);

        // TODO -- This dumps a ton to stdout -- 
        std::cout << "DEBUG -- TODO -- REMOVE ALL THIS PRINTS InjectionPointBaseFactory::parseXMLFILE " << std::endl;
        ifstream ifs(filename.c_str());
        ifs >> *doc;
        parseXMLFile(doc);

    }
    catch(XPlus::Exception& ex) {
        cerr << "  => write failed" << endl;
        cerr << endl << "{" << endl;
        cerr << ex.msg();
        cerr << endl << "}" << endl;
        exit(1);
    }
    
    //FIXME -- ADD TO XML FILE
    InjectionPointBaseFactory::setEngineManager("adios", "todo","todo",false);


}

void InjectionPointBaseFactory::writeIntroduction() {
    InjectionPointBaseFactory::manager->writeIntroduction(intro);
}
void InjectionPointBaseFactory::writeConclusion() {
    InjectionPointBaseFactory::manager->writeConclusion(outro);
}

void InjectionPointBaseFactory::writeXMLFile(std::string filename) {

    try {
        vv::Document * doc = new vv::Document(true);
        doc->prettyPrint(true);
        doc->set_root_exe();
        populateDocument(doc);
        ofstream ofs(filename.c_str());
        ofs << *doc;
        delete doc;
    }
    catch(XPlus::Exception& ex) {
        cerr << "  => write failed" << endl;
        cerr << endl << "{" << endl;
        cerr << ex.msg();
        cerr << endl << "}" << endl;
        exit(1);
    }
}

bool InjectionPointBaseFactory::runTests = false;

bool InjectionPointBaseFactory::VVInit(std::string configFile) {
    
    runTests = true;
    InjectionPointBaseFactory::parseXMLFile(configFile);
    
    InjectionPointBaseFactory::writeIntroduction();
    
    printAllEngines();

    return true;
}

bool InjectionPointBaseFactory::VVFinalize() {

    writeConclusion();
    manager->finalize();
}


VVOutputEngineManager* InjectionPointBaseFactory::manager = NULL;
std::map<std::string, std::pair<maker_ptr *, variable_register_ptr*> , std::less<std::string>> InjectionPointBaseFactory::test_factory;
std::map<std::string, trans_ptr *, std::less<std::string>> InjectionPointBaseFactory::trans_factory;
std::map<std::string, engine_register_ptr*, std::less<std::string>> InjectionPointBaseFactory::registeredEngines;

void InjectionPointBaseFactory::setEngineManager(std::string type, std::string outfileName, std::string configFile, bool debug) {
    manager = (registeredEngines[type])(outfileName,configFile,debug);
}


IVVTransform* InjectionPointBaseFactory::getTransform(std::string name) {

    auto it = InjectionPointBaseFactory::trans_factory.find(name);


    if ( it != InjectionPointBaseFactory::trans_factory.end() ) {
        return it->second();
    }
    return new DefaultVVTransform();
}

InjectionPointRegistrar::InjectionPointRegistrar(std::string scope,  // the name of the injection point
        int injectionIndex,   // the index of the injection point in
        std::string filename, // the filename of the injection point in the loop
        int line,             // the line where the injection point was registered
        std::string desc,     // the json file where the injection point description is located.
        int count,             // the number of parameters in the rest of the list (all strings)
        va_list argp) {

    plist params;
    if ( count % 2 != 0 )
        throw "Count is wrong";

    for ( int i = 0; i < count; i=i+2 ) {

        std::string s1 = va_arg(argp, char*);
        std::string s2 = va_arg(argp, char*);

        params.push_back(std::make_pair(s2,s1));
    }

    auto it = getMap()->find(scope);
    if ( it == getMap()->end()) {


        InjectionPoint p(scope, injectionIndex, filename, line, desc, params);
        getMap()->insert( std::make_pair(scope, p ) );
    } else
        it->second.addStage(injectionIndex, filename, line, desc, params);

}

InjectionPointRegistrar::InjectionPointRegistrar(std::string scope,  // the name of the injection point
        int injectionIndex,   // the index of the injection point in
        std::string filename, // the filename of the injection point in the loop
        int line,             // the line where the injection point was registered
        std::string desc,     // the json file where the injection point description is located.
        int count,             // the number of parameters in the rest of the list (all strings)
        ...) {
    va_list argp;
    va_start(argp, count);

    plist params;
    if ( count % 2 != 0 )
        throw "Count is wrong";


    std::cout << " HEEGSSDGSDGDSG " << count << std::endl;
    for ( int i = 0; i < count; i=i+2 ) {

        std::string s1 = va_arg(argp, char*);
        std::string s2 = va_arg(argp, char*);

        std::cout << " S1 S2 " << s1 << " " << s2 << std::endl;

        params.push_back(std::make_pair(s2,s1));
    }
    va_end(argp);


    auto it = getMap()->find(scope);
    if ( it == getMap()->end()) {


        InjectionPoint p(scope, injectionIndex, filename, line, desc, params);
        getMap()->insert( std::make_pair(scope, p ) );
    } else
        it->second.addStage(injectionIndex, filename, line, desc, params);

}




