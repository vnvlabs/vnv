#define VNV_READER_CACHE_SIZE 100

#include "streaming/MongoNodeImpl.h"

using namespace VnV::Nodes;

mongoc_client_pool_t* Mongo::Mongo::pool;
mongoc_uri_t* Mongo::Mongo::muri;
bson_error_t Mongo::Mongo::error;

// Every thread has its own database
thread_local Mongo::MongoClientWrapper Mongo::Client::clientWrapper;
thread_local Mongo::MongoDatabaseWrapper Mongo::Database::database;
thread_local Mongo::MongoCollectionWrapper Mongo::Collection::thcollection;

std::shared_ptr<DataBase> VnV::Nodes::Mongo::LoadNode(IRootNode* root, long id) {
  auto collection = dynamic_cast<MongoPersistance::RootNode*>(root)->getMainCollection();

  try {
    auto d = collection->getDocument(id, "");

    std::string type = d->get("type").get<std::string>();

    DataBase::DataType t = DataBase::getDataTypeFromString(type);
    std::shared_ptr<DataBase> ptr;
    switch (t) {
#define X(x, y)                                 \
  case DataBase::DataType::x:                   \
    ptr.reset(new MongoPersistance::x##Node()); \
    break;
      DTYPES
#undef X
#define X(x)                                    \
  case DataBase::DataType::x:                   \
    ptr.reset(new MongoPersistance::x##Node()); \
    break;
      STYPES
#undef X
    }

    assert(ptr != nullptr);

    ptr->setRootNode(id, root);
    return ptr;

  } catch (...) {
    std::cout << "SDFSDF" << std::endl;
    return nullptr;
  }
}

template <typename ContainerT, typename PredicateT> void erase_if(ContainerT& items, const PredicateT& predicate) {
  for (auto it = items.begin(); it != items.end();) {
    if (predicate(*it))
      it = items.erase(it);
    else
      ++it;
  }
}

bool Mongo::Collection::persist(long id, json update) {
  json set = json::object();
  set["$set"] = update;
  update_one(id, set);
  return true;
}

void Mongo::Collection::clearCache() {
  auto bulk = mongoc_collection_create_bulk_operation_with_opts(collection(), NULL);
  for (auto& it : docCache) {
    if (it.second->needsPersisted()) {
      it.second->markPersisted();
      auto d = it.second->get();
      json filter = json::object();
      filter["_id"] = it.second->getId();

      json set = json::object();
      set["$set"] = d;

      mongoc_bulk_operation_update_one_with_opts(bulk, bwrap(filter)->get(), bwrap(set)->get(), update_opts, NULL);
    }
  }
  mongoc_bulk_operation_execute(bulk, NULL, &error);
  mongoc_bulk_operation_destroy(bulk);
  docCache.clear();  // TODO;
}

void Mongo::MongoPersistance::WorkflowNode::setReport(std::string reportName, int fileId,
                                                      std::shared_ptr<IRootNode> rootNode) {
  json jj = json::object();
  jj["id"] = rootNode->getId();
  jj["fid"] = fileId;
  jj["collection"] =
      std::dynamic_pointer_cast<MongoPersistance::RootNode>(rootNode)->getMainCollection()->getCollectionName();
  getreports()[reportName] = jj;
  persist();
}

std::shared_ptr<IRootNode> Mongo::MongoPersistance::WorkflowNode::getReport(std::string reportName) {
  auto a = getreports();
  if (a.contains(reportName)) {
    auto root = dynamic_cast<MongoPersistance::RootNode*>(rootNode());
    long id = a[reportName]["id"].get<long>();

    std::string collectionName = a[reportName]["collection"].get<std::string>();
    auto collection = Collection::Initialize(collectionName, root->getMainDatabase());

    return MongoRootNode::load(root->getMainDatabase(), collection);
  }
  return nullptr;
}

std::shared_ptr<Mongo::Document> Mongo::Collection::pullDocument(long id, std::string type, bool clearCache) {
  bool isNew = false;
  auto doc = std::make_shared<Document>(id, loadOrCreate(id, isNew, type), selfAsShared.lock());
  if (isNew) {
    doc->markNew();
  }
  return doc;
}

#define DSTYPES \
  X(Bool, bool) \
  X(Integer, int) X(Float, float) X(Double, double) X(String, std::string) X(Json, std::string) X(Long, long)
#define X(x, y)                                                                                                  \
  y Mongo::MongoPersistance::x##Node::getValueByIndex(const std::size_t ind) { return getvals()[ind].get<y>(); } \
  void Mongo::MongoPersistance::x##Node::add(const y& val) {                                                     \
    auto& s = getvals(true);                                                                                     \
    s.push_back(val);                                                                                            \
  }
DSTYPES
#undef X
#undef DSTYPES

std::shared_ptr<DataBase> Mongo::MongoPersistance::ShapeNode::getValueByIndex(const std::size_t ind) {
  auto a = getvals()[ind].get<long>();
  auto aa = LoadNode(rootNode(), a);
  return aa;
}

void Mongo::MongoPersistance::ShapeNode::add(const std::shared_ptr<DataBase>& s) {
  auto ss = getvals();
  ss.push_back(s->getId());
  setvals(ss);
}
