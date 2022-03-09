
/**
  @file TestStore.h
**/
#ifndef VV_PLUGSSTORE_HEADER
#define VV_PLUGSSTORE_HEADER

#include "base/points/PlugPoint.h"
#include "base/stores/BaseStore.h"
#include "base/stores/TestStore.h"
#include "interfaces/IPlug.h"

namespace VnV {

class PlugsStore : public TestStoreTemplate<IPlug, plug_maker_ptr, PlugConfig>,
                   public BaseStore {
 public:
  PlugsStore() : TestStoreTemplate<IPlug, plug_maker_ptr, PlugConfig>() {}
  static PlugsStore& instance();
  
  nlohmann::json schema(json& packageJson) {
    return schema_(packageJson,"Plugs");
  }

};

}  // namespace VnV

#endif
