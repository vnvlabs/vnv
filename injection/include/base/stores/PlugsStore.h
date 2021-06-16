
/**
  @file TestStore.h
**/
#ifndef VV_PLUGSSTORE_HEADER
#define VV_PLUGSSTORE_HEADER


#include "base/stores/TestStore.h"
#include "interfaces/IPlug.h"
#include "base/points/PlugPoint.h"
namespace VnV {


class PlugsStore : public  TestStoreTemplate<IPlug, plug_maker_ptr, PlugConfig> {
public:
   PlugsStore() : TestStoreTemplate<IPlug, plug_maker_ptr, PlugConfig> (){}

   static PlugsStore& instance() {
     static PlugsStore store;
     return store;
  }
};

} // namespace VnV

#endif
