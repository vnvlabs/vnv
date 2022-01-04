
#include "streaming/dispatch.h"

namespace VnV {
namespace Nodes {

void extract(const json& config, std::string& uri, std::string& db, std::string& coll) {
  uri = config.value("uri", "mongodb://localhost:27017");
  db = config.value("db", "vnv");
  coll = config.value("collection", "hello");
}

std::string dispathSchema() {
  return R"(
      { "type" : "object" , 
        "properties" : {
           "uri" : {"type":"string", "default" : "mongodb://localhost:27017" },
           "db" : {"type":"string", "default" : "vnv" },
           "collection" : {"type":"string", "default" : "hello" },
           "persist" : {"type":"string", "default" : "mongo" }
        } 
     }
    )";
};

}  // namespace Nodes
}  // namespace VnV
