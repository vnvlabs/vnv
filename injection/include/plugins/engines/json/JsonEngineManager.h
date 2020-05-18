#ifndef JSONENGINEMANAGER_H
#define JSONENGINEMANAGER_H

#include "interfaces/IOutputEngine.h"
namespace VnV {
namespace PACKAGENAME {
namespace Engines {

class JsonEngineManager : public OutputEngineManager {

   json mainJson ;
   json::json_pointer ptr;
   long id = 0;
   std::string outputFile;


   std::string getId();

   void append(json& jsonOb);

   void pop(int num);

   void push(json &jsonOb);

   void append(json::json_pointer ptr);

   void push(json &jsonOb, json::json_pointer ptr);

   std::string Dump(int d);

public:
   JsonEngineManager();

   #define LTypes X(double) X(int) X(bool) X(float) X(long) X(std::string) X(json)
   #define X(type) \
   void Put(VnV_Comm comm,std::string variableName, const type& value);
   LTypes
   #undef X
   #undef LTypes

   void Log(VnV_Comm comm,const char *package, int stage,  std::string level, std::string message);

   json getConfigurationSchema();

   void finalize();

    void setFromJson(json& config);

    void injectionPointEndedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageVal);

    void injectionPointStartedCallBack(VnV_Comm comm,std::string id, InjectionPointType type, std::string stageVal);

    void testStartedCallBack(VnV_Comm comm,std::string testName);

    void testFinishedCallBack(VnV_Comm comm,bool result_);

    void unitTestStartedCallBack(VnV_Comm comm,std::string unitTestName);

    void unitTestFinishedCallBack(VnV_Comm comm,std::map<std::string, bool> &results);

    Nodes::IRootNode* readFromFile(std::string file);

    // IInternalOutputEngine interface
    std::string print();
};
}
}
}
#endif // JSONENGINEMANAGER_H
