

/**
  @file OptionsParserStore.h
**/

#ifndef VVOPTIONSPARSER_H
#define VVOPTIONSPARSER_H

#include <map>
#include <string>


#include "base/stores/BaseStore.h"
#include "c-interfaces/Options.h"
#include "interfaces/IOptions.h"

using nlohmann::json;

namespace VnV
{

  class RawJsonObject
  {
  public:
    RawJsonObject(json &j) : data(j) {}
    json data;
  };

  class OptionsParserStore : public BaseStore
  {
    friend class Runtime;

  private:
    std::map<std::string, std::pair<json, std::pair<options_callback_ptr, options_cpp_callback_ptr>>,
             std::less<std::string>>
        factory;

    std::map<std::string, void *> optionResult;

  public:
    OptionsParserStore();

    void add(std::string name, json &m, options_callback_ptr v);
    void add(std::string name, json &m, options_cpp_callback_ptr v);
    void callBack(std::string name, json info, std::vector<std::string> &cmdline, ICommunicator_ptr world);

    void *getResult(std::string name);

    void parse(json info, std::vector<std::string> &cmdline, ICommunicator_ptr world);

    json &getSchema(std::string package);

    static OptionsParserStore &instance();

    json schema(json &packageJson);
  };

} // namespace VnV
#endif // VVOPTIONSPARSER_H
