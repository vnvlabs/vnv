
#include <iostream>

#include "base/exceptions.h"
#include "interfaces/IPipeline.h"
#include "base/stores/PipelineStore.h"
#include "interfaces/IOutputEngine.h"


void VnV::registerPipeline(std::string packageName, std::string name,
                            VnV::pipeline_ptr ptr) {
  PipelineStore::instance().addPipeline(packageName, name, ptr);
}
