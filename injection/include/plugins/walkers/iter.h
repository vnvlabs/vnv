#ifndef VNV_WALKER_ITER_HELPER_HEADER
#define VNV_WALKER_ITER_HELPER_HEADER

#include <map>
#include "interfaces/Nodes.h"

namespace VnV {
namespace Walkers {

class Iter {
 protected:
  VnV::Nodes::ICommMap* commMap;
  std::map<long, std::list<std::tuple<long, long, VnV::Nodes::node_type>>>& nodes;
  std::map<long, std::list<std::tuple<long, long, VnV::Nodes::node_type>>>::iterator niter;
 

  virtual bool parentContainsChild(long parent, long child) {
    return commMap->commContainsComm(parent, child);
  }

  virtual bool commContainsProc(long comm, long proc) {
    return commMap->commContainsProcessor(comm, proc);
  }

 public:
  Iter(Nodes::ICommMap* comm,
        std::map<long, std::list<std::tuple<long, long, VnV::Nodes::node_type>>>& n)
      : commMap(comm), nodes(n) {
    niter = nodes.begin();
  }
};

}  // namespace Walkers
}  // namespace VnV

#endif