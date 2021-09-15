
/**
  @file TestStore.h
**/
#ifndef VV_ITERATORSSTORE_HEADER
#define VV_ITERATORSSTORE_HEADER

/** @file vv-testing.h
 */

#include "base/stores/BaseStore.h"
#include "base/stores/TestStore.h"
#include "interfaces/IIterator.h"

namespace VnV {

class IteratorsStore
    : public TestStoreTemplate<IIterator, iterator_maker_ptr, IteratorConfig>,
      public BaseStore {
 public:
  IteratorsStore()
      : TestStoreTemplate<IIterator, iterator_maker_ptr, IteratorConfig>() {}

  static IteratorsStore& instance();
};

}  // namespace VnV

#endif
