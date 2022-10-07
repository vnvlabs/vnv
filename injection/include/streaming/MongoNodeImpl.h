#ifndef STREAMNODES_MONGO_TEMPLATES_H
#define STREAMNODES_MONGO_TEMPLATES_H
#if MONGO_DB

#  include <mongoc/mongoc.h>

#  include <string>

#  include "base/exceptions.h"
#  include "streaming/common.h"
#  include "streaming/streamtemplate.h"

#  define Mongo_getter_setter(name, type, def)                                  \
    type get##name(bool willmodify = false) {                                   \
      return getDocument()->getOr(#name, def, willmodify).template get<type>(); \
    }                                                                           \
    void set##name(const type& value) { getDocument()->update(#name, value); }

#  define Mongo_getter_setter_json(name, def)                                                         \
    json& get##name(bool willmodify = false) { return getDocument()->getOr(#name, def, willmodify); } \
    void set##name(const json& value) { getDocument()->update(#name, value); }

#  define Mongo_docuemnt_ref(NAME, TYPE)                                                       \
    Mongo_getter_setter(NAME##Id, long, -1) std::shared_ptr<TYPE##Node> getInternal_##NAME() { \
      std::shared_ptr<TYPE##Node> ptr;                                                         \
      ptr.reset(new TYPE##Node());                                                             \
      long idv = get##NAME##Id();                                                              \
      if (idv < 0) {                                                                           \
        rootNode()->registerNode(ptr);                                                         \
        ptr->setname(#NAME);                                                                   \
        set##NAME##Id(ptr->getId());                                                           \
      } else {                                                                                 \
        ptr->setRootNode(idv, (rootNode()));                                                   \
      }                                                                                        \
      return ptr;                                                                              \
    }

namespace VnV {
namespace Nodes {
namespace Mongo {

typedef std::string Id;

class bson_wrap {
  bson_t* b = nullptr;

 public:
  bson_wrap(const json& j) {
    bson_error_t error;
    std::string s = j.dump();
    b = bson_new_from_json((const uint8_t*)s.c_str(), -1, &error);
  }

  bson_wrap(bson_t* bs) { b = bs; }

  bson_t* get() { return b; }

  ~bson_wrap() {
    if (b != nullptr) {
      bson_destroy(b);
    }
  }
};

typedef std::shared_ptr<bson_wrap> bson_shared;

class Mongo {
  // A singleton is used to manage the client pool. Each thread should
  // in theory request only a single client. A Client is returned when
  // the count reaches zero for the client.
  static mongoc_client_pool_t* pool;
  static mongoc_uri_t* muri;
  static bson_error_t error;

  Mongo(std::string uri, std::string name = "VnV") {
    mongoc_init();
    muri = mongoc_uri_new_with_error(uri.c_str(), &error);
    Mongo::pool = mongoc_client_pool_new(muri);
    mongoc_client_pool_set_appname(pool, name.c_str());
  }

  virtual ~Mongo() {
    mongoc_uri_destroy(muri);
    mongoc_client_pool_destroy(pool);
    mongoc_cleanup();
  }

 public:
  static Mongo& instance(std::string uri) {
    static Mongo mongo(uri);
    return mongo;
  }

  mongoc_client_t* getClient() {
    auto c = mongoc_client_pool_pop(pool);
    if (!c) {
      throw INJECTION_EXCEPTION_("Error Creating Mongo Client");
    }
    return c;
  }

  void returnClient(mongoc_client_t* client) { mongoc_client_pool_push(pool, client); }
};

class Collection;
class Database;
class Document;

class MongoClientWrapper {
 private:
  mongoc_client_t* client = NULL;
  std::string uri;

 public:
  void set(std::string nuri) {
    if (client) {
      Mongo::instance(uri).returnClient(client);
    }
    uri = nuri;
    client = Mongo::instance(uri).getClient();
  }

  MongoClientWrapper() {}

  mongoc_client_t* get() { return client; }

  ~MongoClientWrapper() {
    if (client) {
      Mongo::instance(uri).returnClient(client);
    }
  }
};

class Client {
  // Every thread has its own client
  static thread_local MongoClientWrapper clientWrapper;
  std::string uri;

  mongoc_client_t* get_client() {
    if (clientWrapper.get() == NULL) {
      clientWrapper.set(uri);
    }
    return clientWrapper.get();
  }

 public:
  Client(std::string uri, std::string clientName = "VnV") {
    // Launch mongo (no-op except the first time.)
    this->uri = uri;
  }

  mongoc_database_t* getDatabase(std::string db) { return mongoc_client_get_database(get_client(), db.c_str()); }

  virtual ~Client() {}
};

class MongoDatabaseWrapper {
 public:
  mongoc_database_t* database = NULL;

  MongoDatabaseWrapper() {}
  ~MongoDatabaseWrapper() {
    if (database) {
      mongoc_database_destroy(database);
    }
  }
};

class Database {
  std::string name;
  std::shared_ptr<Client> client;

  // Every thread has its own database
  static thread_local MongoDatabaseWrapper database;

  mongoc_database_t* get_database() {
    if (database.database == NULL) {
      database.database = client->getDatabase(name);
    }
    return database.database;
  }

 protected:
 public:
  Database(std::string name, std::shared_ptr<Client> c) {
    this->client = c;
    this->name = name;
  }

  auto getClient() { return client; }

  auto getCollection(std::string collection) {
    return mongoc_database_get_collection(get_database(), collection.c_str());
  }

  virtual ~Database() {}
};

class Cursor {
 public:
  class iterator {
    Cursor* cursor;

   public:
    iterator(Cursor* c) : cursor(c) {}

    Cursor::iterator& operator++() { return cursor->plus(); };

    bool operator==(const iterator& a) { return (cursor == a.cursor); }

    bool operator!=(const iterator& a) { return !(operator==(a)); }

    json asJson() { return cursor->asJson(); }
  };

  Cursor(mongoc_cursor_t* cursor) {
    this->cursor = cursor;
    it.reset(new iterator(this));
  }

  iterator begin() {
    if (!init) {
      init = true;
      return plus();
    }
    throw INJECTION_EXCEPTION_("Cannot call begin twice on Cursor");
  }

  json asJson() {
    char* str = bson_as_json(doc, NULL);
    json j = json::parse(str);
    bson_free(str);
    return j;
  }

  Cursor::iterator& end() {
    static Cursor::iterator end(nullptr);
    return end;
  }

  virtual ~Cursor() {
    if (cursor != nullptr) {
      mongoc_cursor_destroy(cursor);
    }
  }

 private:
  mongoc_cursor_t* cursor = nullptr;
  const bson_t* doc;
  bool init = false;
  std::unique_ptr<iterator> it;
  Cursor() { it.reset(new iterator(this)); }

  iterator& plus() {
    if (mongoc_cursor_next(cursor, &doc)) {
      return *it;
    } else {
      return end();
    }
  }
};

class Mongo_Does_Not_Exist : public std::exception {};
class Mongo_Insert_Failed : public std::exception {};

class MongoCollectionWrapper {
 public:
  mongoc_collection_t* collection = NULL;

  MongoCollectionWrapper() {}
  ~MongoCollectionWrapper() {
    if (collection) {
      mongoc_collection_destroy(collection);
    }
  }
};

class Collection {
  static thread_local MongoCollectionWrapper thcollection;

  std::string collectionName;
  std::shared_ptr<Database> db;
  bson_error_t error;

  mongoc_collection_t* collection_main;
  std::thread::id main_thread_id;
  std::thread::id reader_thread_id;

  bson_t* update_opts;

  std::size_t cacheSize = 1000;

  mongoc_collection_t* collection() {
    if (std::this_thread::get_id() == main_thread_id) {
      return collection_main;
    }

    if (thcollection.collection == nullptr) {
      thcollection.collection = db->getCollection(collectionName);
    }
    return thcollection.collection;
  }

  void insert(std::shared_ptr<bson_wrap> value) {
    if (!mongoc_collection_insert_one(collection(), value->get(), NULL, NULL, &error)) {
      throw Mongo_Insert_Failed();
    }
  }
  auto count(std::shared_ptr<bson_wrap> filter) {
    return mongoc_collection_count_documents(collection(), filter->get(), NULL, NULL, NULL, &error);
  }
  auto remove_one(std::shared_ptr<bson_wrap> filter) {
    return mongoc_collection_delete_one(collection(), filter->get(), NULL, NULL, &error);
  }

  static auto bwrap(json& j) { return std::make_shared<bson_wrap>(j); }

  json get_one(long id) {
    json j = json::object();
    j["id_"] = id;

    auto c = std::make_shared<Cursor>(mongoc_collection_find_with_opts(collection(), bwrap(j)->get(), NULL, NULL));

    if (c->begin() != c->end()) {
      return c->asJson();
    }
    throw Mongo_Does_Not_Exist();
  }

  void update_one(long id, std::shared_ptr<bson_wrap> update) {
    json j = json::object();
    j["id_"] = id;

    mongoc_collection_update_one(collection(), bwrap(j)->get(), update->get(), update_opts, NULL, &error);
  }

  void update_many(std::shared_ptr<bson_wrap> filter, std::shared_ptr<bson_wrap> update) {
    mongoc_collection_update_many(collection(), filter->get(), update->get(), update_opts, NULL, &error);
  }

  std::weak_ptr<Collection> selfAsShared;

  Collection(std::string collection, std::shared_ptr<Database> db) {
    this->db = db;
    this->collectionName = collection;
    collection_main = db->getCollection(collection);
    main_thread_id = std::this_thread::get_id();

    std::string o = R"({"upsert" : true } )";
    update_opts = bson_new_from_json((const uint8_t*)o.c_str(), -1, &error);
  }

  std::size_t getCacheSize() {
    if (readerThread()) {
      return 10000;
    }
    return 0;  // No cache in reader threads -- always update from database.
  }

  std::map<long, std::shared_ptr<Document>> docCache;
  std::map<long, std::weak_ptr<Document>> weakCache;

  std::shared_ptr<Document> addToCache(long id, std::shared_ptr<Document> doc) {
    if (getCacheSize() <= 0) {
      return doc;
    }

    // When the cache is full, we persist everything
    // and clear it out. This makes it batch persists
    // instead of lots of one-offs.
    if (docCache.size() == getCacheSize()) {
      clearCache();
    }

    auto it = docCache.find(id);
    if (it == docCache.end()) {
      docCache[id] = doc;
      weakCache[id] = std::weak_ptr<Document>(doc);
    }
    return doc;
  }

  std::shared_ptr<Document> fetchFromCache(long id, std::string type) {
    if (getCacheSize() > 0) {
      auto it = docCache.find(id);
      if (it != docCache.end()) {
        return it->second;
      } else {
        // Search the weak cache for the object (in case someone else already checked it out)
        auto wit = weakCache.find(id);
        if (wit != weakCache.end()) {
          auto a = wit->second.lock();
          if (a != nullptr) {
            // It was in the weak cache so pass that one through
            return a;
          } else {
            // It was in the cache but the shared obj was deleted.
            // This should not happen ?
            weakCache.erase(wit);
          }
        }
      }
    }

    auto doca = pullDocument(id, type);
    return addToCache(id, doca);
  }

 public:
  static std::shared_ptr<Collection> Initialize(std::string collection, std::shared_ptr<Database> db) {
    std::shared_ptr<Collection> p;
    p.reset(new Collection(collection, db));
    p->selfAsShared = p;
    return p;
  }

  std::string getCollectionName() { return collectionName; }

  void clearCache();

  void set_reader_thread() { reader_thread_id = std::this_thread::get_id(); }

  bool mainThread() { return std::this_thread::get_id() == main_thread_id; }
  bool readerThread() { return reader_thread_id == std::this_thread::get_id(); }

  void removeFromWeakCache(long id) {
    if (readerThread()) {
      weakCache.erase(id);
    }
  }

  void update_one(long id, json& update) { update_one(id, bwrap(update)); }

  std::shared_ptr<Cursor> find(json& selector, json& opts) {
    auto sel = std::make_shared<bson_wrap>(selector);
    auto b = std::make_shared<bson_wrap>(opts);
    return std::make_shared<Cursor>(mongoc_collection_find_with_opts(collection(), sel->get(), b->get(), NULL));
  }

  auto count(const json& filter) { return count(std::make_shared<bson_wrap>(filter)); }

  // The reader thread calls get_one to find out if the id exists -- But, 99.99% of the
  // time, it doesnt exist (cause we are adding a new node.) By keeping track of the largest
  // id we have seen, and based on the fact that ids are mostly sequential, we can skip get_one
  // in all these cases.
  long largest_seen = -1;

  json createEntry(long id, bool& isNew, std::string type) {
    if (type.empty()) {
      throw INJECTION_EXCEPTION_("Type empty when does not exist");
    }
    json filter = json::object();
    filter["id_"] = id;
    filter["type"] = type;
    filter["metadata"] = "{}";
    isNew = true;
    return filter;
  }

  json loadOrCreate(long id, bool& isNew, std::string type = "") {
    if (readerThread() && id > largest_seen) {
      largest_seen = id;
      return createEntry(id, isNew, type);
    }

    try {
      json filter = json::object();
      filter["_id"] = id;
      auto h = get_one(id);
      isNew = false;
      return h;
    } catch (Mongo_Does_Not_Exist e) {
      return createEntry(id, isNew, type);
    }
  }

  // return the number of items in the collection
  std::size_t size() { return count(json::object()); }

  // Not really a cache -- This keeps the Document around as long
  // as someone else is using it -- Pretty cool.

  std::mutex m;

  std::shared_ptr<Document> pullDocument(long idv, std::string type, bool clearCache = false);

  std::shared_ptr<Document> getDocument(long id, std::string type) {
    assert(id > -1 && "The ID has not been set ");
    return fetchFromCache(id, type);
  }

  bool persist(long id, json update);

  bool remove(long id) {
    json filter = json::object();
    filter["_id"] = id;
    remove_one(bwrap(filter));
    return true;
  }

  virtual ~Collection() {
    bson_destroy(update_opts);
    mongoc_collection_destroy(collection_main);
  }
};

typedef std::shared_ptr<Collection> Collection_ptr;

class Document {
  long id;
  json doc;
  bool updated = false;
  bool isNew = false;
  bool deleted = false;

 public:
  std::shared_ptr<Collection> collection;

  // Will create a new Document if it doesn't exist.
  Document(long id, const json& doc, std::shared_ptr<Collection> collection) {
    this->collection = collection;
    this->id = id;
    this->doc = doc;
  }

  void markNew() { isNew = true; }
  void markUpdated() { updated = true; }
  void markDeleted() {
    deleted = true;
    if (!isNew && collection->readerThread()) {
      collection->remove(id);
    }
  }

  bool needsPersisted() {
    // If marked deleted then we return false.
    if (deleted) {
      return false;
    }
    return updated || isNew;
  }

  void markPersisted() {
    updated = false;
    isNew = false;
  }

  ~Document() {
    if (collection->readerThread()) {
      collection->removeFromWeakCache(id);
      persist();

    } else {
      // Auto persist on reader thread only. If the gui threads want to persist
      // they should do so explicitly
    }
  }

  long getId() { return id; }

  json& get() { return doc; }

  bool persist(bool hard = false) {
    if (needsPersisted()) {
      collection->persist(id, doc);
      markPersisted();
      return true;
    }
    return false;
  }

  json& get(std::string field, bool willModify = false) {
    if (doc.contains(field)) {
      if (willModify) {
        updated = true;
      }

      return doc[field];
    }

    throw INJECTION_EXCEPTION("Field %s does not exist:", field.c_str());
  }

  json& getOr(std::string field, const json& value, bool willModify = false) {
    if (willModify) {
      updated = true;
    }

    if (doc.contains(field)) {
      return doc[field];
    }

    doc[field] = value;
    return doc[field];
  }

  void update(std::string field, const json& value) {
    markUpdated();
    try {
      doc[field] = value;
    } catch (std::exception& e) {
      std::cout << doc.dump(4) << std::endl;
      std::cout << value.dump(4) << std::endl;
      std::cout << "Update failed: " << e.what();
    }
  }
};

typedef std::shared_ptr<Database> Database_ptr;
typedef std::shared_ptr<Client> Client_ptr;

std::shared_ptr<DataBase> LoadNode(IRootNode* root, long id);

class MongoPersistance {
 public:
  template <typename T> class DataBaseImpl : public T {
   private:
    std::shared_ptr<Document> doc;

   protected:
    bool autop = false;

    Collection_ptr getCollection() {
      return dynamic_cast<MongoPersistance::RootNode*>(T::rootNode())->getMainCollection();
    }

    Document* getDocument() {
      if (doc == nullptr) {
        doc = getCollection()->getDocument(T::getId(), T::getTypeStr());
      }
      return doc.get();
    }

   public:
    DataBaseImpl() : T() {}
    virtual ~DataBaseImpl() {}

    Mongo_getter_setter(streamId, long, -1) Mongo_getter_setter(name, std::string, "")
        Mongo_getter_setter(open_, bool, false)

    void setmetadata(MetaDataWrapper& meta) {
      getDocument()->update("metadata", meta.toJson());
    }

    void markReaderThread() { getCollection()->set_reader_thread(); }

    void persist() { getDocument()->persist(); }

    void throwAway() { getDocument()->markDeleted(); }

    MetaDataWrapper w;
    const MetaDataWrapper& getmetadata() {
      w.fromJson(getDocument()->getOr("metadata", json::object()));
      return std::move(w);
    }

    // Override interface for DataBase class.
    virtual const MetaDataWrapper& getMetaData() override { return getmetadata(); };
    virtual std::string getNameInternal() override { return getname(); }
    virtual bool getopen() override { return getopen_(); };
    virtual void setopen(bool value) override { setopen_(value); }
    virtual long getStreamId() override { return getstreamId(); }
  };

  class ArrayNode : public DataBaseImpl<IArrayNode> {
    std::map<long, std::shared_ptr<DataBase>> nodes;

   public:
    ArrayNode()
        : DataBaseImpl<IArrayNode>(){}

          ;
    Mongo_getter_setter_json(vec, json::array());

    virtual std::shared_ptr<DataBase> get(std::size_t idx) override {
      auto& a = getvec();
      if (idx < a.size()) {
        long nid = a[idx].get<long>();
        auto it = nodes.find(nid);
        return LoadNode(rootNode(), a[idx].get<long>());
      }
      return nullptr;
    }

    virtual std::size_t size() override { return getvec().size(); };

    virtual void add(std::shared_ptr<DataBase> data) override {
      auto& a = getvec(true);
      a.push_back(data->getId());
    }
  };

  class MapNode : public DataBaseImpl<IMapNode> {
   public:
    MapNode()
        : DataBaseImpl<IMapNode>(){}

          ;
    Mongo_getter_setter_json(map, json::object());

    template <typename T> bool _convertToShapeNode(std::shared_ptr<DataBase> parent, std::shared_ptr<DataBase> child) {
      if (convertToShapeNode<T>(parent, child)) {
        auto a = std::dynamic_pointer_cast<T>(child);
        a->throwAway();
        return true;
      }
      return false;
    }


    virtual void insert(std::string key, std::shared_ptr<DataBase> val) {
      auto& m = getmap(true);

      if (m.contains(key)) {
        auto edbn = LoadNode(rootNode(), m[key].get<long>());
        auto metadata = val->getMetaData();
        std::string collate = metadata.has("collate") ? metadata.get("collate") : "shape";

        // If replace then we just override it.
        if (collate.compare("replace") == 0) {
          m[key] = val->getId();
        } else if (edbn->getType() != DataType::Array) {
          // Not an array -- so convert it (we collate ids of same type.)
          auto a = std::make_shared<ArrayNode>();
          rootNode()->registerNode(a);
          a->setname(key);
          a->add(edbn);
          a->add(val);
          m[key] = a->getId();
        } else {
          // Add it to the array.
          auto an = edbn->getAsArrayNode(edbn);
          an->add(val);
        }
      } else {
        m[key] = val->getId();
      }
    }

    virtual std::shared_ptr<DataBase> get(std::string key) override {
      auto& m = getmap();
      if (m.contains(key)) {
        return LoadNode(rootNode(), m[key].get<long>());
      }
      throw INJECTION_EXCEPTION("Key %s does not exist in the map:", key.c_str());
    }

    virtual bool contains(std::string key) override { return getmap().contains(key); }

    virtual std::vector<std::string> fetchkeys() override {
      std::vector<std::string> v;
      for (auto it : getmap().items()) {
        v.push_back(it.key());
      }
      return v;
    }

    virtual std::size_t size() override { return getmap().size(); };

    virtual ~MapNode(){};
  };

#  define X(x, y)                                                                                   \
    class x##Node : public DataBaseImpl<I##x##Node> {                                               \
      Mongo_getter_setter_json(shape, json::array()) Mongo_getter_setter_json(vals, json::array())  \
                                                                                                    \
          public : x##Node()                                                                        \
          : DataBaseImpl<I##x##Node>() {}                                                           \
      std::vector<std::size_t> ss;                                                                  \
      const std::vector<std::size_t>& getShape() override {                                         \
        ss = convertArrayToVector<std::size_t>(getshape());                                         \
        return ss;                                                                                  \
      }                                                                                             \
                                                                                                    \
      y getValueByShape(const std::vector<std::size_t>& rshape) override {                          \
        auto shape = getshape();                                                                    \
        if (shape.size() == 0) {                                                                    \
          return getValueByIndex(0);                                                                \
        }                                                                                           \
        if (rshape.size() != shape.size())                                                          \
          throw INJECTION_EXCEPTION("%s: Invalid Shape Size %d (should be %d)",                     \
                #x, rshape.size(), shape.size());                                                   \
                                                                                                    \
        std::size_t mult = 1;                                                                       \
        int index = 0;                                                                              \
        for (int i = shape.size()-1; i >= 0 ; i--) {                                                \
           index += rshape[i] * mult;                                                               \
           mult *= shape[i].get<std::size_t>();                                                     \
      }                                                                                             \
        return  getValueByIndex(index);                                                             \
      }                                                                                             \
      void add(const y& v);                                                                         \
      y getValueByIndex(const size_t ind) override;                                                 \
                                                                                                    \
      y getScalarValue() override {                                                                 \
        if (getshape().size() == 0) {                                                               \
          return getValueByIndex(0);                                                                \
        } else {                                                                                    \
          throw INJECTION_EXCEPTION("%s: No shape provided to non scalar shape tensor object", #x); \
        }                                                                                           \
      }                                                                                             \
                                                                                                    \
      int getNumElements() override { return calculateNumElements<std::size_t>(getshape()); }       \
      virtual ~x##Node() {}                                                                         \
      virtual void setShape(const std::vector<std::size_t>& s) { setshape(s); }                     \
      virtual void setValue(const std::vector<y>& s) {                                              \
        setvals(json::array());                                                                     \
        for (auto ot : s) {                                                                         \
          add(ot);                                                                                  \
        }                                                                                           \
      }                                                                                             \
    };
  DTYPES
#  undef X

  class InfoNode : public DataBaseImpl<IInfoNode> {
   public:
    InfoNode() : DataBaseImpl<IInfoNode>() {}

    Mongo_getter_setter(title, std::string, "");
    Mongo_getter_setter(workflow, std::string, "");
    Mongo_getter_setter(jobName, std::string, "");
    Mongo_getter_setter(start, long, 0);
    Mongo_getter_setter(end, long, 0);

    Mongo_getter_setter_json(prov, json::object());

    virtual std::string getTitle() override { return gettitle(); }

    virtual std::string getWorkflow() override { return getworkflow(); }

    virtual std::string getJobName() override { return getjobName(); }

    void setprov(std::shared_ptr<VnVProv> p) { setprov(p->toJson()); }

    virtual long getStartTime() override { return getstart(); }
    virtual long getEndTime() override { return getend(); }

    virtual std::shared_ptr<VnVProv> getProvInternal() override { return std::make_shared<VnVProv>(getprov()); }

    virtual void addInputFile(std::shared_ptr<VnV::ProvFile> pv) override {
      auto a = getProvInternal();
      a->addInputFile(pv);
      setprov(a->toJson());
    }

    virtual void addOutputFile(std::shared_ptr<VnV::ProvFile> pv) override {
      auto a = getProvInternal();
      a->addOutputFile(pv);
      setprov(a->toJson());
    }

    virtual ~InfoNode() {}
  };

  class CommInfoNode : public DataBaseImpl<ICommInfoNode> {
    std::shared_ptr<CommMap> commMap;

   protected:
    virtual std::shared_ptr<ICommMap> getCommMapInternal() override {
      if (commMap == nullptr) {
        commMap.reset(new CommMap());
        auto a = getcommMapData();
        for (auto it : getcommMapData().items()) {
          std::set<long> c;
          for (auto itt : it.value().items()) {
            c.insert(itt.value().get<long>());
          }
          commMap->add(std::atol(it.key().c_str()), c);
        }
      }
      return commMap;
    }

   public:
    CommInfoNode() : DataBaseImpl<ICommInfoNode>() {}

    Mongo_getter_setter(worldSize, int, -1);
    Mongo_getter_setter(version, std::string, "");
    Mongo_getter_setter_json(nodeMap, json::object());
    Mongo_getter_setter_json(commMapData, json::object());

    virtual int getWorldSize() override { return getworldSize(); }
    virtual std::string getNodeMap() override { return getnodeMap().dump(); }
    virtual std::string getVersion() override { return getversion(); }

    virtual void add(long id, const std::set<long>& comms) override {
      // Get the commmap (inits it if not inited yet)
      auto cm = getCommMapInternal();

      // Get the data and add this to it.
      auto a = getcommMapData();
      a[std::to_string(id)] = comms;

      // update the data;
      setcommMapData(a);
      // Update the cm manually (could also reset and reload)
      cm->add(id, comms);
    }

    virtual ~CommInfoNode(){};
  };

  class WorkflowNode : public DataBaseImpl<IWorkflowNode> {
   private:
   public:
    Mongo_getter_setter_json(info, json::object());
    Mongo_getter_setter_json(reports, json::object());

    Mongo_getter_setter(package, std::string, "");
    Mongo_getter_setter(state, std::string, "");

    WorkflowNode() : DataBaseImpl<IWorkflowNode>() {}
    virtual std::string getPackage() override { return getpackage(); }
    virtual std::string getState() override { return getstate(); }
    virtual json getInfo() override { return getinfo(); }

    virtual std::shared_ptr<IRootNode> getReport(std::string reportName) override;

    virtual bool hasReport(std::string reportName) override { return getreports().contains(reportName); }

    virtual void setReport(std::string reportName, int fileId, std::shared_ptr<IRootNode> rootNode) override;

    virtual std::vector<std::string> listReports() override {
      std::vector<std::string> ret;
      for (auto& it : getreports().items()) {
        ret.push_back(it.key());
      }
      return ret;
    }

    virtual int getReportFileId(std::string reportName) {
      if (hasReport(reportName)) {
        return getreports()[reportName]["fid"].get<int>();
      }
      return -2200;
    }
  };

  class TestNode : public DataBaseImpl<ITestNode> {
   private:
    Mongo_docuemnt_ref(logs, Array) Mongo_docuemnt_ref(data, Map)

        public : Mongo_getter_setter(package, std::string, "") Mongo_getter_setter(usage, std::string, "Test")
                     Mongo_getter_setter(internal, bool, false) Mongo_getter_setter(result, bool, false)
                         Mongo_getter_setter(uid, long, -1) Mongo_getter_setter_json(frequest, json::object());

    TestNode() : DataBaseImpl<ITestNode>() {}

    virtual bool isInternal() override { return getinternal(); }

    virtual std::string getPackage() override { return getpackage(); }

    virtual ITestNode::TestNodeUsage getUsage() override { return ITestNode::getUsageFromString(getusage()); }

    void setusage(ITestNode::TestNodeUsage u) { setusage(ITestNode::getUsageString(u)); }

    virtual std::shared_ptr<IMapNode> getData() override { return getInternal_data(); }

    virtual std::shared_ptr<IArrayNode> getLogs() override { return getInternal_logs(); }

    virtual std::shared_ptr<FetchRequest> getFetchRequest() override {
      if (fetch != nullptr) {
        return fetch;
      }

      json& j = getfrequest();

      if (j.size() > 0) {
        std::string schema = j["s"].get<std::string>();
        long id = j["i"].get<long>();
        long jid = j["j"].get<long>();
        long expiry = j["e"].get<long>();
        std::string message = j["m"].get<std::string>();
        fetch.reset(new FetchRequest(schema, id, jid, expiry, message));

        return fetch;
      }

      return nullptr;
    }

    virtual void resetFetchRequest() {
      fetch.reset();
      setfrequest(json::object());
    }

    virtual void setFetchRequest(std::string schema, long id, long jid, long expiry, std::string message) {
      json j = json::object();
      j["s"] = schema;
      j["i"] = id;
      j["j"] = jid;
      j["e"] = expiry;
      j["m"] = message;
      setfrequest(j);
    }
  };

  class InjectionPointNode : public DataBaseImpl<IInjectionPointNode> {
   public:
    Mongo_docuemnt_ref(logs, Array);
    Mongo_docuemnt_ref(tests, Array);
    Mongo_docuemnt_ref(internal, Test);

    Mongo_getter_setter(package, std::string, "");
    Mongo_getter_setter(commId, long long, -1);
    Mongo_getter_setter(startIndex, long, -1);

    Mongo_getter_setter(endIndex, long, -1);
    Mongo_getter_setter(isIter, bool, false);
    Mongo_getter_setter(isOpen, bool, false);

    InjectionPointNode() : DataBaseImpl<IInjectionPointNode>() {}

    void setinternal(std::shared_ptr<TestNode> test) { setinternalId(test->getId()); }

    virtual std::shared_ptr<TestNode> getinternal() { return getInternal_internal(); }

    virtual std::string getPackage() override { return getpackage(); }

    virtual std::shared_ptr<IArrayNode> getTests() override { return getInternal_tests(); }

    virtual std::shared_ptr<ITestNode> getData() override { return getInternal_internal(); }

    virtual std::shared_ptr<IArrayNode> getLogs() override { return getInternal_logs(); }

    virtual std::string getComm() override { return std::to_string(getcommId()); }

    virtual long getStartIndex() override { return getstartIndex(); }
    virtual long getEndIndex() override { return getendIndex(); }

    std::shared_ptr<TestNode> getTestByUID(long uid) {
      auto a = getTests();

      for (int i = 0; i < a->size(); i++) {
        auto ba = a->get(i);
        assert(ba->getType() == DataBase::DataType::Test);
        auto t = std::dynamic_pointer_cast<TestNode>(ba);
        if (t->getuid() == uid) {
          return t;
        }
      }

      std::cout << "Could not find one " << a->size() << " " << uid << std::endl;

      return nullptr;
    }

    virtual ~InjectionPointNode() {}
  };

  class LogNode : public DataBaseImpl<ILogNode> {
   public:
    Mongo_getter_setter(package, std::string, "") Mongo_getter_setter(level, std::string, "")
        Mongo_getter_setter(stage, std::string, "") Mongo_getter_setter(message, std::string, "")
            Mongo_getter_setter(comm, std::string, "") Mongo_getter_setter(identity, int, -1)

                LogNode()
        : DataBaseImpl<ILogNode>() {}
    virtual std::string getPackage() override { return getpackage(); }
    virtual std::string getLevel() override { return getlevel(); }
    virtual std::string getMessage() override { return getmessage(); }
    virtual std::string getComm() override { return getcomm(); }
    virtual std::string getStage() override { return getstage(); }
    virtual ~LogNode() {}
  };

  class DataNode : public DataBaseImpl<IDataNode> {
    Mongo_docuemnt_ref(logs, Array) Mongo_docuemnt_ref(children, Map)

        public : Mongo_getter_setter(local, bool, false) Mongo_getter_setter(key, long long, -1)
                     Mongo_getter_setter(package, std::string, "")

                         DataNode()
        : DataBaseImpl<IDataNode>() {}

    virtual bool getLocal() override { return getlocal(); }
    virtual long long getDataTypeKey() override { return getkey(); }

    virtual std::shared_ptr<IMapNode> getData() override { return getInternal_children(); }

    virtual std::shared_ptr<IArrayNode> getLogs() override { return getInternal_logs(); };

    virtual ~DataNode() {}
  };

  class UnitTestResultNode : public DataBaseImpl<IUnitTestResultNode> {
   public:
    Mongo_getter_setter(desc, std::string, "") Mongo_getter_setter(result, bool, false)

        UnitTestResultNode()
        : DataBaseImpl<IUnitTestResultNode>() {}
    virtual std::string getDescription() override { return getdesc(); }
    virtual bool getResult() override { return getresult(); }
    virtual ~UnitTestResultNode() {}
  };

  class UnitTestResultsNode : public DataBaseImpl<IUnitTestResultsNode> {
    Mongo_docuemnt_ref(m, Map);

    auto getM() { return getInternal_m(); }

   public:
    UnitTestResultsNode() : DataBaseImpl<IUnitTestResultsNode>() {}

    virtual std::shared_ptr<IUnitTestResultNode> get(std::string key) {
      if (getM()->contains(key)) {
        auto a = getM()->get(key);
        return a->getAsUnitTestResultNode(a);
      }
      throw INJECTION_EXCEPTION("Unit Tests Results Node: Key %s does not exist:", key.c_str());
    };

    void insert(std::string name, std::shared_ptr<IUnitTestResultNode> value) { getM()->insert(name, value); }

    virtual bool contains(std::string key) { return getM()->contains(key); }

    virtual std::vector<std::string> fetchkeys() { return getM()->fetchkeys(); };

    virtual ~UnitTestResultsNode(){};
  };

  class UnitTestNode : public DataBaseImpl<IUnitTestNode> {
    Mongo_docuemnt_ref(logs, Array) Mongo_docuemnt_ref(children, Map) Mongo_docuemnt_ref(resultsMap, UnitTestResults)

        public :

        Mongo_getter_setter(package, std::string, "")

            UnitTestNode()
        : DataBaseImpl<IUnitTestNode>() {}

    virtual std::string getPackage() override { return getpackage(); }

    virtual std::shared_ptr<IMapNode> getData() override { return getInternal_children(); }

    virtual std::shared_ptr<IArrayNode> getLogs() override { return getInternal_logs(); };

    virtual std::shared_ptr<IUnitTestResultsNode> getResults() override { return getInternal_resultsMap(); }
  };

  class RootNode : public DataBaseImpl<IRootNode> {
    Collection_ptr collection;
    Database_ptr database;
    std::atomic<bool> _processing = ATOMIC_VAR_INIT(true);

    Mongo_docuemnt_ref(children, Array) Mongo_docuemnt_ref(logs, Array) Mongo_docuemnt_ref(unitTests, Array)
        Mongo_docuemnt_ref(actions, Map) Mongo_docuemnt_ref(packages, Map)
            Mongo_docuemnt_ref(initialization,Test)
            Mongo_docuemnt_ref(infoNode, Info) Mongo_docuemnt_ref(commInfoNode, CommInfo)
                Mongo_docuemnt_ref(workflowNode, Workflow)

                    std::shared_ptr<VnVSpec> spec;
    Mongo_getter_setter_json(specdata, json::object());

    auto getSpecInternal() {
      if (spec == nullptr) {
        spec.reset(new VnVSpec(getspecdata()));
      }
      return spec;
    }

   public:
    Mongo_getter_setter(lowerId, long, -1);
    Mongo_getter_setter(upperId, long, -1);

    Mongo_getter_setter_json(idn, json::object());

    Collection_ptr getMainCollection() { return collection; }
    Database_ptr getMainDatabase() { return database; }

    void setMainCollection(Database_ptr db, Collection_ptr c) { database = db, collection = c; }

    RootNode() : DataBaseImpl<IRootNode>() {
      autop = true;  // Root Node should autopersist because it might not get deleted.
    }

    virtual ~RootNode() {}

    void cache_persist(bool clearCache) { collection->clearCache(); }

    void setinfoNode(std::shared_ptr<IInfoNode> node) { setinfoNodeId(node->getId()); }
    void setworkflowNode(std::shared_ptr<IWorkflowNode> node) { setworkflowNodeId(node->getId()); }

    void setProcessing(bool value) { _processing.store(value, std::memory_order_relaxed); }

    virtual void respond(long id, long jid, const std::string& response) override {}

    virtual std::shared_ptr<IMapNode> getPackages() override { return getInternal_packages(); }

    virtual std::shared_ptr<ITestNode> getInitialization() override { return getInternal_initialization(); }

    virtual std::shared_ptr<IMapNode> getActions() override { return getInternal_actions(); }

    virtual std::shared_ptr<IArrayNode> getChildren() override { return getInternal_children(); }

    virtual std::shared_ptr<IArrayNode> getUnitTests() override { return getInternal_unitTests(); }

    virtual std::shared_ptr<IInfoNode> getInfoNode() override { return getInternal_infoNode(); }

    virtual std::shared_ptr<IArrayNode> getLogs() override { return getInternal_logs(); }

    virtual std::shared_ptr<ICommInfoNode> getCommInfoNode() override { return getInternal_commInfoNode(); }

    virtual std::shared_ptr<IWorkflowNode> getWorkflowNode() override { return getInternal_workflowNode(); }

    virtual void persist() override { getDocument()->persist(true); }

    void setspec(const json& s) {
      getSpecInternal()->set(s);
      setspecdata(s);
    }

    virtual bool processing() const override { return _processing.load(); }

    virtual std::shared_ptr<DataBase> findById_Internal(long id) override {
      try {
        return LoadNode(rootNode(), id);
      } catch (std::exception& e) {
        throw INJECTION_EXCEPTION("Error Loading Internal Node: Invalid Id %s", id);
      }
    }

    std::map<long, std::list<IDN>> idMap;
    virtual std::map<long, std::list<IDN>>& getNodes() override {
      if (idMap.size() == 0) {
        auto& j = getidn();
        for (auto it : j.items()) {
          long ind = std::atol(it.key().c_str());

          std::list<IDN> n;
          for (auto& itt : it.value().items()) {
            n.push_back(IDN(itt.value()));
          }
          idMap[ind] = n;
        }
      }
      return idMap;
    }

    virtual void registerNodeInternal(std::shared_ptr<DataBase> ptr) override {
      // Registration is not needed -- We dont save nodes.
    }

    virtual void addIDN(long id, long streamId, node_type type, long index, std::string stage) override {
      json newIDN = json::object();
      newIDN["id"] = id;
      newIDN["streamId"] = streamId;
      newIDN["type"] = Node_Type_To_Int(type);
      newIDN["stage"] = stage;

      json& idn = getidn();

      if (!idn.contains(std::to_string(index))) {
        idn[std::to_string(index)] = {newIDN};
        getNodes()[index] = {IDN(newIDN)};
      } else {
        (idn[std::to_string(index)]).push_back(newIDN);
        getNodes()[index].push_back(IDN(newIDN));
      }
    }

    virtual const VnVSpec& getVnVSpec() { return *(getSpecInternal()); }
  };
};

template <typename T, typename V>
class MongoRootNodeWithThread : public StreamParserTemplate<MongoPersistance>::RootNodeWithThread<T, V> {
 public:
  Client_ptr client;
  Database_ptr db;
  Collection_ptr collection;

  MongoRootNodeWithThread(std::string uri, std::string dbname, std::string collname, bool allowExisting = false) {
    client.reset(new Client(uri));
    db.reset(new Database(dbname, client));
    collection = Collection::Initialize(collname, db);

    if (!allowExisting && collection->size() > 0) {
      throw INJECTION_EXCEPTION("Cannot Load into an existing collection %s. Please provide a unique collection name.",
                                collname.c_str());
    }

    // collection->setCacheSize(1000);
    MongoPersistance::RootNode::setMainCollection(db, collection);
  }

  static std::shared_ptr<IRootNode> parse(
      bool async, std::string uri, std::string dbname, std::string collname, std::shared_ptr<T> stream,
      std::shared_ptr<StreamParserTemplate<MongoPersistance>::ParserVisitor<V>> visitor = nullptr) {
    auto root = std::make_shared<MongoRootNodeWithThread>(uri, dbname, collname);

    root->stream = stream;
    root->registerNode(root);
    root->setname("Root Node");

    root->visitor =
        (visitor == nullptr) ? std::make_shared<StreamParserTemplate<MongoPersistance>::ParserVisitor<V>>() : visitor;
    root->visitor->set(stream, root.get());
    root->open(true);

    root->run(async);
    return root;
  }

  virtual ~MongoRootNodeWithThread() {}
};

class MongoRootNode : public MongoPersistance::RootNode {
 public:
  MongoRootNode() {}

  void lock() override {}
  void release() override {}
  virtual void respond(long id, long jid, const std::string& response) override {}

  static std::shared_ptr<IRootNode> load(Database_ptr db, Collection_ptr collection) {
    auto root = std::make_shared<MongoRootNode>();
    root->setMainCollection(db, collection);
    root->registerNode(root);
    root->setProcessing(false);
    root->open(false);
    return root;
  }

  static std::shared_ptr<IRootNode> load(std::string uri, std::string dbname, std::string collname) {
    auto client = std::make_shared<Client>(uri);
    auto db = std::make_shared<Database>(dbname, client);
    auto collection = Collection::Initialize(collname, db);
    return load(db, collection);
  }
};

typedef StreamParserTemplate<MongoPersistance> MongoParser;

template <typename N> using MongoParserVisitor = MongoParser::ParserVisitor<N>;

template <typename N> using NoLockMongoParserVisitor = MongoParser::NoLockParserVisitor<N>;

}  // namespace Mongo
}  // namespace Nodes
}  // namespace VnV

#endif
#endif