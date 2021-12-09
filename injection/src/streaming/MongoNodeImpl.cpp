#define VNV_READER_CACHE_SIZE 25

#include "streaming/MongoNodeImpl.h"

using namespace VnV::Nodes;

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

template< typename ContainerT, typename PredicateT >
  void erase_if( ContainerT& items, const PredicateT& predicate ) {
    for( auto it = items.begin(); it != items.end(); ) {
      if( predicate(*it) ) it = items.erase(it);
      else ++it;
    }
}


bool Mongo::Collection::persistAll() {
    
    //This persists all valid cache items and removes any that are
    //no longer valid.
    std::lock_guard<std::mutex> hold(m);
    erase_if(cache, []( std::pair<const long, std::weak_ptr<Document>>& item ) {
       if (item.second.expired()) {
         return true;
       } else {
         item.second.lock()->persist();
         return false;
       }
    });
    return true;

  }


std::shared_ptr<Mongo::Document> Mongo::Collection::pullDocument(long id, std::string type, bool clearCache) {

  // This is just a basic cache storing shared pointers. A shared pointer exists in memeory until
  // it is removed from the cache and everyone is done with it. 
  class SimpleCache {
    int max_size;
    std::queue<std::shared_ptr<VnV::Nodes::Mongo::Document>> q;

   public:
    SimpleCache(std::size_t msize) : max_size(msize) {}
    std::shared_ptr<VnV::Nodes::Mongo::Document> add(std::shared_ptr<VnV::Nodes::Mongo::Document> d) {
      //q.push(d);
      //if (q.size() > max_size) q.pop();
      return d;
    }

    void clear() {
      std::queue<std::shared_ptr<VnV::Nodes::Mongo::Document>>().swap(q);
    }
  };

  static SimpleCache s(VNV_READER_CACHE_SIZE);
  
  if (clearCache) {
    s.clear();
    return nullptr;
  }
  
  
  return s.add(std::make_shared<Document>(id, loadOrCreate(id, type), selfAsShared.lock()));
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
