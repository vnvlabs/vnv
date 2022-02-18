#define VNV_READER_CACHE_SIZE 25

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
}


template <typename ContainerT, typename PredicateT> void erase_if(ContainerT& items, const PredicateT& predicate) {
  for (auto it = items.begin(); it != items.end();) {
    if (predicate(*it))
      it = items.erase(it);
    else
      ++it;
  }
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
    
    return MongoRootNode::load(root->getMainDatabase(),collection);
  }
  return nullptr;
}

std::shared_ptr<Mongo::Document> Mongo::Collection::pullDocument(long id, std::string type, bool clearCache) {
  return std::make_shared<Document>(id, loadOrCreate(id, type), selfAsShared.lock());
}

#define DSTYPES \
  X(Bool, bool) \
  X(Integer, int) X(Float, float) X(Double, double) X(String, std::string) X(Json, std::string) X(Long, long)
#define X(x, y)                                                                                                  \
  y Mongo::MongoPersistance::x##Node::getValueByIndex(const std::size_t ind) { return getvals()[ind].get<y>(); } \
  void Mongo::MongoPersistance::x##Node::add(const y& val) {                                                     \
    auto s = getvals();                                                                                          \
    s.push_back(val);                                                                                            \
    setvals(s);                                                                                                  \
  }
DSTYPES
#undef X
#undef DSTYPES

std::shared_ptr<DataBase> Mongo::MongoPersistance::ShapeNode::getValueByIndex(const std::size_t ind) {
  auto it = nodes.find(ind);
  if (it == nodes.end()) {
    auto a = getvals()[ind].get<long>();
    auto aa = LoadNode(rootNode(), a);
    nodes[ind] = aa;
    return aa;
  }
  return it->second;
}

void Mongo::MongoPersistance::ShapeNode::add(const std::shared_ptr<DataBase>& s) {
  auto ss = getvals();
  nodes[ss.size()] = s;
  ss.push_back(s->getId());
  setvals(ss);
}
