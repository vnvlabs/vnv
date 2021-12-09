#include "streaming/IDN.h"

using namespace VnV::Nodes;

int VnV::Nodes::Node_Type_To_Int(node_type t) { 
  switch (t)
  {
  case node_type::ROOT: return 0;
  case node_type::POINT: return 1;
  case node_type::START: return 2;
  case node_type::ITER: return 3;
  case node_type::END: return 4;
  case node_type::LOG: return 5;
  case node_type::WAITING: return 6;
  case node_type::DONE: return 7; 
  }
  throw VnV::VnVExceptionBase("code issue 11232344");
}

node_type VnV::Nodes::Node_Type_From_Int(int i) {
  switch (i)
  {
  case 0: return node_type::ROOT;
  case 1: return node_type::POINT;
  case 2: return node_type::START;
  case 3: return node_type::ITER;
  case 4: return node_type::END;
  case 5: return node_type::LOG;
  case 6: return node_type::WAITING;
  case 7: return node_type::DONE; 
  }
  throw VnV::VnVExceptionBase("code issue 11232344a");

}