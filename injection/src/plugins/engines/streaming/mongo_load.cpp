#include <unistd.h>

#if MONGO_DB

#  include "streaming/dispatch.h"

using namespace VnV::Nodes::Mongo;

INJECTION_ENGINE_READER(VNVPACKAGENAME, mongo, VnV::Nodes::dispathSchema()) {
  std::string uri, dbname, coll;
  extract(config, uri, dbname, coll);
  return MongoRootNode::load(uri, dbname, coll);
}

#endif