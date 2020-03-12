
#ifndef VV_DOCUMENTATIONSTORE_H
#define VV_DOCUMENTATIONSTORE_H


#include <map>
#include <string>

#include "interfaces/ITest.h"


namespace VnV {

class DocumentationStore {

 private:
  std::map<std::string, std::map<std::string, std::string>> document_factory;
  DocumentationStore();

 public:
  void addDocumentation(std::string packageName, std::string name, std::map<std::string, std::string> &map);

  VnVParameterSet getParameterMap(std::string pname, std::string doc, std::map<std::string, std::pair<std::string,void*>> &m);

  static DocumentationStore& instance();

  void registerDocumentation(std::string packageName, std::string jsonStr);

};


}


#endif
