#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include <map>
#include <string>
#include <regex>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "base/stores/TransformStore.h"
#include "json-schema.hpp"

using nlohmann::json;


namespace VnV {

enum TestStatus { SUCCESS, FAILURE, NOTRUN };

enum class InjectionPointType { Single, Begin, End, Iter };
namespace InjectionPointTypeUtils {
std::string getType(InjectionPointType type, std::string stageId);
int toC(InjectionPointType type);
} 

enum class VariableEnum { Double, String, Int, Float, Long };
namespace VariableEnumFactory {
  VariableEnum fromString(std::string s);
  std::string toString(VariableEnum e);
}  // namespace VariableEnumFactory

typedef std::map<std::string, std::string> MetaData;




class OutputEngineManager;

class VnVParameter {
  void* ptr;
  std::string type;
  bool input = true;

  std::shared_ptr<Transformer> trans = nullptr;
  std::string transType = "";

 public:
  VnVParameter() { ptr = nullptr; }

  VnVParameter(void* obj, std::string type_, bool input_) {
    type = type_;
    ptr = obj;
    input = input_;
  }

  VnVParameter(const VnVParameter& copy) {
    ptr = copy.getRawPtr();
    type = copy.getType();
    input = copy.isInput();
  }

  void setType(std::string type) { this->type = type; }

  void setInput(bool input) { this->input = input; }

  void* getRawPtr() const { return ptr; }

  std::string getType() const { return type; }

  bool isInput() const { return input; };

  template <typename T> T* getPtr(std::string requestedType) {
    StringUtils::squash(type);

    if (trans == nullptr || type.compare(transType) != 0) {
      trans = TransformStore::instance().getTransformer(type, requestedType);
      transType = requestedType;
    }

    if (trans != nullptr && type.compare(transType) == 0) {
      return static_cast<T*>(trans->Transform(ptr));
    }
    throw VnVExceptionBase("Bad Transform Requested -- Cannot transform from %s -> %s", type.c_str(),
                           requestedType.c_str());
  }

  template <typename T> T& getRef(std::string type) { return *getPtr<T>(type); }
};

class VnVParameterSet : public std::map<std::string, VnVParameter> {
 public:
  template <typename T> T* getInputPtr(std::string name, std::string type) { return getPtr<T>(name, type, true); }

  template <typename T> T* getOutputPtr(std::string name, std::string type) { return getPtr<T>(name, type, false); }

  template <typename T> T& getInputRef(std::string name, std::string type) { return getRef<T>(name, type, true); }

  template <typename T> T& getOutputRef(std::string name, std::string type) { return getRef<T>(name, type, false); }

  template <typename T> T* getPtr(std::string name, std::string type) { return getPtr<T>(name, type, true); }
  template <typename T> T& getRef(std::string name, std::string type) { return getRef<T>(name, type, true); }


  template <typename T> T* getPtr(std::string name, std::string type, bool input) {
    StringUtils::squash(type);
    auto it = find(name);
    if (it != end()) {
      if (it->second.isInput() != input) {
        if (input) {
          throw VnVExceptionBase("Requested an input parameter but got an output parameter");
        }
        throw VnVExceptionBase("Requested an output parameter but got an input parameter");
      }
      return it->second.getPtr<T>(type);
    }
    throw VnVExceptionBase("Parameter Mapping Error.");
  }

  template <typename T> T& getRef(std::string name, std::string type, bool input) {
    StringUtils::squash(type);
    auto it = find(name);
    if (it != end()) {
      if (it->second.isInput() != input) {
        if (input) {
          throw VnVExceptionBase("Requested an input parameter but got an output parameter");
        }
        throw VnVExceptionBase("Requested an output parameter but got an input parameter");
      }
      return it->second.getRef<T>(type);
    }
    throw VnVExceptionBase("Parameter Mapping Error.");
  }
};


class TestConfig {
 protected:
  std::map<std::string, std::shared_ptr<Transformer>> transformers;
  VnVParameterSet* parameters;

  std::string testName;
  std::string package;
  nlohmann::json testConfigJson;
  nlohmann::json runConfig;

 public:
  TestConfig(std::string package, std::string name, json& usersConfig);
  json runTemplateName = json::object();

  /**
   * @brief getAdditionalParameters
   * @return
   */
  const json& getAdditionalParameters() const;

  /**
   * @brief setName
   * @param name
   */
  void setName(std::string name);

  VnVParameterSet& getParameterMap();

  json& getRunConfig() { return runConfig;}

  void setParameterMap(VnVParameterSet& args);

  /**
   * @brief getName
   * @return
   */
  std::string getName() const;
  std::string getPackage() const;
  /**
   * @brief print out configuration information.
   */
  void print();
};

typedef std::function<void(VnV_Comm comm, VnV::VnVParameterSet& ntv, VnV::OutputEngineManager* engine,
                           VnV::InjectionPointType type, std::string stageId)> DataCallback;

void defaultCallBack(VnV_Comm comm, VnV::VnVParameterSet& ntv, IOutputEngine* engine, InjectionPointType type,
                     std::string stageId);


}



#endif
