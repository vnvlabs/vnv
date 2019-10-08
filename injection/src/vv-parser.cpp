

#include <string>
#include <iostream>
#include "vv-parser.h"

using namespace VnV;


IParser* ParserStore::getParser(std::string fileExtension) {
  auto it = registeredParsers.find(fileExtension);
  if ( it != registeredParsers.end() ) {
    return it->second(fileExtension);
  }
  throw "No parser registered for that file extension";
}

ParserStore::ParserStore(){}

ParserStore& ParserStore::getParserStore() {
  static ParserStore *parser = new ParserStore();
  return *parser;
}

void ParserStore::registerParser(std::string name, parser_register_ptr* parser_ptr) {
  registeredParsers.insert(std::make_pair(name,parser_ptr));
}

void VnV_registerParser(std::string name, parser_register_ptr *r) { 
  ParserStore::getParserStore().registerParser(name,r);  
}




