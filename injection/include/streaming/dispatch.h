#ifndef STREAMING_DISPATCH_H
#define STREAMING_DISPATCH_H

#include "streaming/MongoNodeImpl.h"
#include "streaming/InMemoryNodeImpl.h"

using namespace VnV::Nodes;

namespace VnV {
namespace Nodes {


void extract(const json& config, std::string &uri, std::string &db, std::string &coll);

template<typename T, typename V> 
std::shared_ptr<IRootNode> engineReaderDispatch(bool async, const json& config, std::shared_ptr<T> stream, bool lock = true) {
  
  std::string db = config.value("persist","mongo");
  #if MONGO_DB 
  if ( db.compare("mongo") == 0 ) {
    
    std::string uri, dbname, coll;
    extract(config,uri,dbname,coll); 
    
    auto pv = lock ? std::make_shared<Mongo::MongoParserVisitor<V>>()  : 
                     std::make_shared<Mongo::NoLockMongoParserVisitor<V>>(); 
                          
    return Mongo::MongoRootNodeWithThread<T, json>::parse(async, uri,dbname,coll, stream, pv);
  } 
  #endif 

  auto pv = lock ? std::make_shared<Memory::InMemoryParserVisitor<V>>()  : 
                   std::make_shared<Memory::NoLockInMemoryParserVisitor<V>>(); 
    
  return Memory::InMemoryRootNodeWithThread<T, json>::parse(async, stream, pv);
}

}
}


#endif
