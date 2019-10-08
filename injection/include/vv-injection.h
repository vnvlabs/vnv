
#ifndef VV_INJECTION_POINTS_H
#define VV_INJECTION_POINTS_H

#include <string>
#include <map>
#include <vector>
#include <stdarg.h>

// Forward Declarations. 

namespace VnV {

class TestConfig;
class ITest;

// TypeDefs 
typedef std::vector<std::pair<std::string,std::string>> plist;
typedef std::map<std::string, std::pair<std::string,void*> > NTV;

class InjectionPoint {

  private:  
    std::string m_scope;
    std::vector< ITest* > m_tests;
    
    void unpack_parameters( NTV &ntv, va_list argp);
  public:

    InjectionPoint(std::string scope, int stage, std::string filename, int lineNumber, std::string desc, plist &params);
    InjectionPoint(std::string scope);
    std::string getScope() const;
    void runTests(int ipType, va_list argp);
    void addTest(TestConfig c);
    bool hasTests();
};

typedef std::map<std::string, InjectionPoint > ip_map;

class InjectionPointStore {
private:  
    std::map<std::string, InjectionPoint*> injectionPoints; 
    InjectionPointStore();

public:    
    InjectionPoint* getInjectionPoint(std::string key);
    
    void addInjectionPoint(std::string scope, InjectionPoint *point);
    
    static InjectionPointStore& getInjectionPointStore();
};

}


#endif

