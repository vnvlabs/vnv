#ifndef JSONCPPINTERFACE_H
#define JSONCPPINTERFACE_H

#include "common-interfaces/PackageName.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"

using nlohmann::json;
namespace VnV {

typedef void* (*options_cpp_callback_ptr)(json& info, std::vector<std::string>& cmdline, IOutputEngine* engine, ICommunicator_ptr world);

void RegisterOptions(std::string packageName, std::string schema, options_cpp_callback_ptr callback);

void RegisterOptions_Json(std::string packageName, json& schema, options_cpp_callback_ptr callback);

void* getOptionsObject(std::string packageName);

}  // namespace VnV

#ifdef __cplusplus

#  define INJECTION_OPTIONS(PNAME, schema, OptionsObject)                                                            \
    namespace VnV {                                                                                                  \
    namespace PNAME {                                                                                                \
    OptionsObject* optionsCallback_(json& config, std::vector<std::string>& cmdline, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world);         \
    void* optionsCallback(json& config, std::vector<std::string>& cmdline, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world) {                  \
      return (void*)optionsCallback_(config, cmdline, engine, world);                                                         \
    }                                                                                                                \
    void registerOptions() { VnV::RegisterOptions(VNV_STR(PNAME), schema, &optionsCallback); }                       \
    OptionsObject* getOptionsObject() { return static_cast<OptionsObject*>(VnV::getOptionsObject(VNV_STR(PNAME))); } \
    }                                                                                                                \
    }                                                                                                                \
    OptionsObject* VnV::PNAME::optionsCallback_(json& config, std::vector<std::string>& cmdline, VnV::IOutputEngine* engine, VnV::ICommunicator_ptr world)

#  define DECLAREOPTIONS(PNAME) \
    namespace VnV {             \
    namespace PNAME {           \
    void registerOptions();     \
    }                           \
    }

#  define REGISTEROPTIONS(PNAME) VnV::PNAME::registerOptions();

#  define REGISTERRAWOPTIONS(PNAME, SCHEMA) VnV::RegisterOptions(VNV_STR(PNAME), SCHEMA, nullptr);

#  define INJECTION_GET_CONFIG(PNAME) VnV::PNAME::getOptionsObject();

#endif

#endif  // JSONINTERFACE_H
