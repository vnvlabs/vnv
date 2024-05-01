#include "base/CommMapper.h"
#include "shared/exceptions.h"

using namespace VnV;

std::shared_ptr<InjectionPointMerger> InjectionPointMerger::join(
      std::string outputfile, std::set<CommWrap_ptr>& comms,
      std::function<std::shared_ptr<InjectionPointInterface>(long)>& parse) {
    INJECTION_ASSERT((comms.size() != 1), "Invalid Comms Object. Size should be one but it was %ld", comms.size());

    std::map<long, CommWrap_ptr> commsMap = CommMapper::convertToMap(comms);
    std::shared_ptr<InjectionPointMerger> dstruct = nullptr;
    std::set<long> done;
    for (auto it : commsMap) {
      join(dstruct, it.second->id, commsMap, outputfile, done, parse);
    }
    return dstruct;
  }

