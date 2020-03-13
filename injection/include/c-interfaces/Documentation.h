#ifndef DOCUMENTATION_H
#define DOCUMENTATION_H

#include "c-interfaces/PackageName.h"

// Simple functionality. This allows users to place a single VNV_DOCUMENT macro at any
// point in the code. This macro pings the VnVRuntime and indicates that a documentation
// object should be written at this point.
// The functionality of VnV_DOcument is to write the variables and link them to
// the documentation that was entered in the source code. Assuming we have the type information (from
// parsing the source code), we can also link the types to a serializer for automated writing.


#ifdef __cplusplus
#include "interfaces/CppInjection.h"
namespace VnV {
namespace CppDocumentation {

//Utility class to ask a varialbe to be serialized using a particular serializer.
class Serialize {
public:
    template<typename T>
    Serialize(T ptr, std::string serializer) {
        ptr = static_cast<void*>(&ptr);
        name = serializer;
    }
    void* ptr;
    std::string name;
};

void Document(const char* package, const char * id, std::map<std::string,std::pair<std::string,void*>> &map);

template<typename ...Args>
void DocumentPack(const char* package, const char * name, Args&&...args) {
    std::map<std::string,std::pair<std::string,void*>> m;
    CppInjection::UnwrapParameterPack(m,std::forward<Args>(args)...);
    Document(package, name, m);
}

void RegisterDocumentation(std::string packageName, std::string name, std::map<std::string, std::string> &m);

}

}

#define DOIT_DOC(X) , #X, X
#define EVERYONE_DOC(...) FOR_EACH(DOIT_DOC,__VA_ARGS__)
#define VNV_DOCUMENT(name) VnV::CppDocumentation::DocumentPack(VNV_STR(PACKAGENAME),#name) ;
#define VNV_DOCUMENT_P(name, ...) VnV::CppDocumentation::DocumentPack(VNV_STR(PACKAGENAME), #name EVERYONE_DOC(__VA_ARGS__));



#else

#define VNV_DOCUMENT(name) _VnV_Document(VNV_STR(PACKAGENAME),#name, __FILE__,__LINE__, VNV_END_PARAMETERS_S ) ;

#define DOIT_DOC(X) , #X, (void*) &X
#define EVERYONE_DOC(...) FOR_EACH(DOIT_DOC,__VA_ARGS__)
#define VNV_DOCUMENT_P(name, ...) _VnV_Document(VNV_STR(PACKAGENAME), #name, __FILE__,__LINE__ EVERYONE_DOC(__VA_ARGS__), VNV_END_PARAMETERS_S);

VNVEXTERNC void _VnV_Document(const char *pname, const char* name, const char *file, int line, ...);

#endif

#define Register_Documentation(documentationJson) \
    _VnV_Register_Documentation(VNV_STR(PACKAGENAME), documentationJson);
VNVEXTERNC void _VnV_Register_Documentation(const char *packageName, const char * jsonString);

#endif // DOCUMENTATION_H
