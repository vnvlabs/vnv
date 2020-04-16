
#include "c-interfaces/Documentation.h"
#include "base/Runtime.h"
#include "base/Utilities.h"
#include "base/DocumentationStore.h"

using nlohmann::json;

void VnV::CppDocumentation::Document(VnV_Comm comm, const char* package, const char * id, std::map<std::string,std::pair<std::string,void*>> &map) {
    VnV::RunTime::instance().documentationPoint(comm, package, id, map);
}


extern "C" {

void _VnV_Document(VnV_Comm comm, const char *packageName, const char *name, ...) {

   va_list args;
   va_start(args, name);
   auto ntv = VnV::VariadicUtils::UnwrapVariadicArgs(args);
   VnV::RunTime::instance().documentationPoint(comm, packageName, name, ntv);
   va_end(args);

}

void _VnV_Register_Documentation(const char * packageName, const char * jsonString) {
    VnV::DocumentationStore::instance().registerDocumentation(packageName, jsonString);
}

}


