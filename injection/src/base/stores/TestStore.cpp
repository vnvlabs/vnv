
/** @file TestStore.cpp **/

#include "base/stores/TestStore.h"

#include <dlfcn.h>

#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include "base/Runtime.h"
#include "base/parser/JsonSchema.h"
#include "base/stores/OutputEngineStore.h"
#include "c-interfaces/Logging.h"
#include "json-schema.hpp"

using nlohmann::json_schema::json_validator;

using namespace VnV;

BaseStoreInstance(TestStore)
