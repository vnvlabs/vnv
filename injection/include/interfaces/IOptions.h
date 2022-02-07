#ifndef JSONCPPINTERFACE_H
#define JSONCPPINTERFACE_H


#include "c-interfaces/PackageName.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"


using nlohmann::json;
namespace VnV {

  typedef void* (*options_cpp_callback_ptr)(json& info, IOutputEngine* engine, ICommunicator_ptr world);
  
  void RegisterOptions(std::string packageName, std::string schema, options_cpp_callback_ptr callback);
  
  void RegisterOptions_Json(std::string packageName, json& schema, options_cpp_callback_ptr callback);

}  // namespace VnV

#define INJECTION_OPTIONS(PNAME, schema)                                                         \
   namespace VnV {                                                                                \
   namespace PNAME {                                                                              \
     void* optionsCallback(json& config, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world); \
     void registerOptions() { VnV::RegisterOptions(VNV_STR(PNAME), schema, &optionsCallback); }     \
   }                                                                                              \
   }                                                                                              \
   void* VnV::PNAME::optionsCallback(json& config, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world)

#define DECLAREOPTIONS(PNAME) \
   namespace VnV {             \
   namespace PNAME {           \
     void registerOptions();     \
   }                           \
   }

#define REGISTEROPTIONS(PNAME) VnV::PNAME::registerOptions();


#  endif  // JSONINTERFACE_H
