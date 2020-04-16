#ifndef ITEST_H
#define ITEST_H

#include <map>
#include <string>
#include "json-schema.hpp"
#include "interfaces/IOutputEngine.h"
#include "base/TransformStore.h"
#include "base/exceptions.h"
#include "c-interfaces/Logging.h"
using nlohmann::json;
/**
 * @brief The TestConfig class
 */
namespace VnV {

enum TestStatus { SUCCESS, FAILURE, NOTRUN };



class VnVParameter {
    void* ptr;
    std::string rtti;
    std::string type;
    bool hasRtti = true;

 public:

    VnVParameter() {
        ptr = nullptr;
    }

    VnVParameter( void* obj, std::string type_) {
        rtti = "";
        type = type_;
        ptr = obj;
        hasRtti = false;
    }

    VnVParameter(const VnVParameter &copy) {
        ptr = copy.getRawPtr();
        rtti = copy.getRtti();
        type = copy.getType();
        hasRtti = copy.hasRtti;
    }

    VnVParameter(void* obj, std::string type_, std::string rtti_) {
        rtti = rtti_;
        type = type_;
        ptr = obj;
        hasRtti = (rtti.size()>0);
    }

    void setType(std::string type){
        this->type = type;
    }

    void setRtti(std::string rtti){
        this->rtti = rtti;
    }

    void* getRawPtr() const {
        return ptr;
    }

    std::string getType() const {
        return type;
    }

    std::string getRtti() const {
        return rtti;
    }

    template<typename T>
    T* getPtr(std::string type, bool checkRtti) const  {
        if ( !type.empty() && getType().compare(type)!=0 ) {
            throw VnVExceptionBase("type information incorrect");
        }
       if (hasRtti && checkRtti) {
            T* tempPtr = static_cast<T*>(getRawPtr());
            std::string typeId = typeid(tempPtr).name();
            if (typeId.compare(rtti) != 0) {
                VnV_Warn("Unmatched RTTI %s: %s", typeId.c_str(), rtti.c_str());
                throw VnVExceptionBase("Rtti information does not match");
            } else {
                return tempPtr;
            }
       }
       return static_cast<T*>(getRawPtr());
    }

    template<typename T>
    T& getRef(std::string type, bool checkRtti) const {
        return *getPtr<T>(type, checkRtti);
    }

};
typedef std::map<std::string,VnVParameter> VnVParameterSet;

class TestConfig {
 private:

  std::map<std::string, std::shared_ptr<Transformer> > transformers;
  VnVParameterSet parameters;

  std::string testName;
  json testConfigJson;
  json testDeclarationJson;


 public:

  TestConfig(std::string name, json &usersConfig, json &testSpec);

  
  bool isRequired(std::string parmaeterName) const;
  /**
   * @brief getAdditionalParameters
   * @return
   */
  const json& getAdditionalParameters() const;


  bool preLoadParameterSet(std::map<std::string,std::string> &parameters) ;


  const json& getExpectedResult() const;
  /**
   * @brief setName
   * @param name
   */
  void setName(std::string name);

  const std::map<std::string,VnVParameter>& getParameterMap() const  ;

  void setParameterMap(std::map<std::string, VnVParameter> &args);
  
  /**
   * @brief getName
   * @return
   */
  std::string getName() const ;

  /**
   * @brief print out configuration information.
   */
  void print();

};


/**
 * @brief The ITest class
 */
class ITest {
 public:
  /**
   * @brief ITest
   */
  ITest(TestConfig &config);


  /**
   * @brief ~ITest
   */
  virtual ~ITest();

  /**
   * @brief _runTest
   * @param engine
   * @param stageVal
   * @param params
   * @return
   */
  TestStatus _runTest(VnV_Comm comm, OutputEngineManager *engine, InjectionPointType type, std::string stageId);

  /**
   * @brief runTest
   * @param engine
   * @param stage
   * @param params
   * @return
   */
  virtual TestStatus runTest(VnV_Comm comm, OutputEngineManager* engine, InjectionPointType type, std::string stageId) = 0;

  /**
   * @brief getConfigurationJson
   * @return
   */
  const json& getConfigurationJson() const ;

  /**
   * @brief getExpectedResultJson
   * @return
   */
  const json& getExpectedResultJson() const ;


  template <typename T>
  T& getReference(std::string name, std::string type) const {
      auto it = m_config.getParameterMap().find(name);
      if ( it!= m_config.getParameterMap().end()) {
        return it->second.getRef<T>(type,true);
      }
      throw VnVExceptionBase("Parameter Mapping Error.");
  }

private:
  const TestConfig m_config;

  /**
   * @brief carefull_cast
   * @tparam T class type
   * @param stage Test Stage
   * @param parameterName The name of the parameter to convert
   * @param parameters The Map of parameters passed to the Test by the injection
   * point.
   */


};

// Search in s for a VnVParameter named "name". Convert the raw ptr to class T with checking.
#define GetRef(a,name,T) T& a = getReference<T>(name,#T);



typedef ITest* maker_ptr(TestConfig config);
typedef json declare_test_ptr();
void registerTest(std::string name, VnV::maker_ptr m, VnV::declare_test_ptr v);

}

#endif // ITEST_H
