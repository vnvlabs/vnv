#include <unistd.h>

#if MONGO_DB 

#include "streaming/dispatch.h"

using namespace VnV::Nodes::Mongo;

namespace {

class MongoRootNode : public MongoPersistance::RootNode {
      
      Client_ptr client;
      Database_ptr db;
      Collection_ptr collection;
     
      MongoRootNode(std::string uri, std::string dbname, std::string collname) {
         client.reset(new Client(uri));
         db.reset(new Database(dbname, client));
         collection = Collection::Initialize(collname, db);
         setMainCollection(collection);
         json j = json::object();
        
      }
      
      virtual void respond(long id, long jid, const std::string& response) override {}

    public:
    
      static std::shared_ptr<IRootNode> load(std::string uri, std::string dbname, std::string collname) {
          std::shared_ptr<IRootNode> root;
          root.reset(new MongoRootNode(uri, dbname, collname));
          root->registerNode(root);
          return root;
      }
};


}

INJECTION_ENGINE_READER(VNVPACKAGENAME, mongo) {
    std::string uri, dbname, coll;
    extract(config,uri,dbname,coll); 
    return MongoRootNode::load(uri,dbname,coll);
}

#endif