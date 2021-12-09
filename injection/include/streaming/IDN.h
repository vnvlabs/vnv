#ifndef IDN_HEADER
#define IDN_HEADER

#include <string>
#include "base/exceptions.h"
#include "json-schema.hpp"
#include "c-interfaces/PackageName.h"
namespace VnV {
namespace Nodes {



enum class node_type { ROOT, POINT, START, ITER, END, LOG, WAITING, DONE };

int Node_Type_To_Int(node_type t);
node_type Node_Type_From_Int(int i);

class IDN {
 public:
  long id;
  long streamId;
  node_type type;
  long duration;
  std::string stage;

  IDN(long a, long b, node_type c, long d, std::string e) : id(a), streamId(b), type(c), duration(d), stage(e) {}
  
  IDN() : id(-1), streamId(-1), type(node_type::ROOT), duration(-1), stage("") {}
  
  IDN(nlohmann::json& j) {
    id = j["id"].get<long>();
    streamId = j["streamId"].get<long>();
    duration = j["duration"].get<long>();
    stage = j["stage"].get<std::string>();
    type = Node_Type_From_Int(j["type"].get<int>());
  }

};

}
}

#endif