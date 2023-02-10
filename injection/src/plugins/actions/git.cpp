
#include "VnV.h"

const char* github_schema = R"(
{
   "type" : "object",
   "properties" : {
       "owner" : {"type" : "string" },
       "repo" : {"type" : "string" },
       "labels": { "type" : "array" , "items" : {"type" : "string"} }
   },
   "required" : ["owner","repo"]
}
)";

const char* gitlab_schema = R"(
{
   "type" : "object",
   "properties" : {
       "base" : {"type" : "string" },
       "id" : {"type" : "integer" },
       "labels": { "type" : "array" , "items" : {"type" : "string"} }
   },
   "required" : ["base","id"]
}
)";

const char* local_schema = R"(
{
   "type" : "array",
   "items" : {
        "type" : "object",
        "properties" : {
            "title" : {"type" : "string" },
            "description" : {"type" : "string" },
            "name" : {"type" : "string" },
            "state" : {"type" : "string", "enum" : ["open","closed"] },
            "date": { "type" : "integer" }
        },
        "required" : []
    }
}
)";

class IssuesAction : public VnV::IAction {
  json conf;

 public:
  IssuesAction(const json& config) { conf = config; }

  virtual void initialize() override { getEngine()->Put("conf", conf); }
};

/**
 * .. vnv-issues:: conf
 *
 *
 *
 */
INJECTION_ACTION(VNVPACKAGENAME, github, github_schema) { return new IssuesAction(config); }

/**
 * .. vnv-issues:: conf
 *   :host: gitlab
 *
 */
INJECTION_ACTION(VNVPACKAGENAME, gitlab, gitlab_schema) { return new IssuesAction(config); }

/**
 * .. vnv-issues:: conf
 *   :host: local
 */
INJECTION_ACTION(VNVPACKAGENAME, issues, local_schema) { return new IssuesAction(config); }
