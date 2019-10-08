#ifndef VV_PARSER_HEADER
#define VV_PARSER_HEADER

#include <string>
#include <set>
#include "vv-testing.h"
namespace VnV {

struct EngineInfo {
    std::string engineType;
    std::string engineConfig;
    std::string outFileName;
    bool debug; 
};

struct RunInfo { 

  
  bool runTests;
  std::set<std::string> testLibraries; 
  std::map<std::string, std::vector<TestConfig>> injectionPoints;

  EngineInfo engineInfo;

  bool error;
  std::string errorMessage;
};

class IParser {
public:
  virtual RunInfo parse(std::string filename) = 0;
};


typedef IParser *parser_register_ptr(std::string);

class ParserStore {
private:
  std::map<std::string, parser_register_ptr*> registeredParsers;
  ParserStore();    
  
public:
  void registerParser(std::string name, parser_register_ptr* parser_ptr);
  IParser* getParser(std::string fileExtension);
  static ParserStore & getParserStore();
};

}

#ifdef __cplusplus
    #define EXTERNC extern "C" 
#else
    #define EXTERNC 
#endif

EXTERNC void VnV_registerParser(std::string name, VnV::parser_register_ptr r); 

#undef EXTERNC

#endif

