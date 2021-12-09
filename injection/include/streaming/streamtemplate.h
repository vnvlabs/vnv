#ifndef ENGINE_CONSTANTS_HEADER
#define ENGINE_CONSTANTS_HEADER
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <list>
#include <mutex>
#include <stack>
#include <string>
#include <thread>

#include "base/ActionType.h"
#include "base/DistUtils.h"
#include "base/Runtime.h"
#include "streaming/Nodes.h"
#include "streaming/curl.h"

namespace VnV {
namespace Nodes {
#define STREAM_IS_CLOSED -199

namespace JSD {

constexpr auto outputFile = "outputFile";

#define NTYPES   \
  X(id)          \
  X(name)        \
  X(package)     \
  X(value)       \
  X(shape)       \
  X(node)        \
  X(meta)        \
  X(comm)        \
  X(children)    \
  X(prov)        \
  X(duration)    \
  X(key)         \
  X(stage)       \
  X(message)     \
  X(internal)    \
  X(description) \
  X(result)      \
  X(stageId)     \
  X(mpiversion)  \
  X(input)       \
  X(reader)      \
  X(level)       \
  X(dtype)       \
  X(endid)       \
  X(line)        \
  X(filename)    \
  X(results)     \
  X(time)        \
  X(spec) X(commList) X(testuid) X(sdt) X(title) X(nodeMap) X(worldSize) X(date)
#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES

}  // namespace JSD

namespace JSN {
#define NTYPES                 \
  X(log)                       \
  X(shape)                     \
  X(dataTypeStarted)           \
  X(dataTypeEnded)             \
  X(injectionPointStarted)     \
  X(injectionPointEnded)       \
  X(injectionPointIterStarted) \
  X(injectionPointIterEnded)   \
  X(packageOptionsStarted)     \
  X(packageOptionsFinished)    \
  X(actionStarted)             \
  X(actionFinished)            \
  X(fetch)                     \
  X(fetchSuccess)              \
  X(fetchFail)                 \
  X(testStarted) X(file) X(done) X(duration) X(testFinished) X(unitTestStarted) X(unitTestFinished) X(commInfo) X(info)

#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES
}  // namespace JSN

namespace JST {
#define NTYPES X(Double) X(Long) X(String) X(Json) X(Bool) X(GlobalArray)
#define X(a) constexpr auto a = #a;
NTYPES
#undef X
#undef NTYPES
}  // namespace JST

class JsonElement {
 public:
  long id;
  json data;
  JsonElement(long i, const json& d) : id(i), data(d) {}
};

template <typename V> class Iterator {
 private:
  std::pair<V, long> current;

  std::stack<std::shared_ptr<DataBase>> stack;
  bool peaked = false;
  bool donedone = false;

  virtual void getLine(V& currentJson, long& currentValue) = 0;

 public:
  Iterator(){};
  virtual long streamId() const = 0;
  virtual bool hasNext() = 0;

  virtual bool start_stream_reader() { return true; };
  virtual void stop_stream_reader(){};

  virtual bool isDone() { return donedone; };

  virtual std::pair<V, long> next() {
    pullLine(false);
    return current;
  }

  void pullLine(bool peek) {
    if (!peaked && hasNext()) {
      getLine(current.first, current.second);

      if (!current.first.contains(JSD::node)) {
        throw VnV::VnVExceptionBase("Stream sent info without node %s", current.first.dump());
      }
      if ((current.first)[JSD::node].template get<std::string>() == JSN::done) {
        donedone = true;
      }
    }
    peaked = peek;
  }

  virtual long peekId() {
    pullLine(true);
    return current.second;
  };

  virtual void respond(long id, long jid, const json& response) { throw VnVExceptionBase("Not implemented Error"); }

  virtual ~Iterator(){};

  virtual void push(std::shared_ptr<DataBase> d) { stack.push(d); }
  virtual std::shared_ptr<DataBase> pop() {
    if (stack.size() > 0) {
      auto s = stack.top();
      stack.pop();
      return s;
    }
    return nullptr;
  }
  virtual std::shared_ptr<DataBase> top() {
    if (stack.size() > 0) {
      return stack.top();
    }
    return nullptr;
  }
};

template <typename T, typename V> class MultiStreamIterator : public Iterator<V> {
  std::list<std::shared_ptr<T>> instreams;
  typename std::list<std::shared_ptr<T>>::iterator min;

  virtual void getLine(V& current, long& cid) override {
    min = std::min_element(instreams.begin(), instreams.end(),
                           [](const std::shared_ptr<T>& x, const std::shared_ptr<T>& y) {
                             if (x->isDone() && y->isDone())
                               return (x->peekId() < y->peekId());
                             else if (x->isDone())
                               return false;
                             else if (y->isDone())
                               return true;

                             return (x->peekId() < y->peekId());
                           });

    auto p = (*min)->next();
    current = p.first;
    cid = p.second;
  }

 public:
  MultiStreamIterator() : Iterator<V>(){};

  std::list<std::shared_ptr<T>>& getInputStreams() { return instreams; }

  virtual void add(std::shared_ptr<T> iter) { instreams.push_back(iter); }

  virtual bool isDone() {
    updateStreams();

    if (instreams.size() == 0) {
      return false;
    }

    for (auto it : instreams) {
      if (!it->isDone()) {
        return false;
      }
    }
    return true;
  }

  virtual bool hasNext() override {
    updateStreams();
    for (auto it : instreams) {
      if (it->hasNext()) {
        return true;
      }
    }
    return false;
  }

  virtual void updateStreams() {}

  virtual long streamId() const override { return (*min)->streamId(); }

  virtual long peekId() {
    this->pullLine(true);
    return (*min)->peekId();
  }

  void push(std::shared_ptr<DataBase> d) override { (*min)->push(d); }

  std::shared_ptr<DataBase> pop() override { return (*min)->pop(); }

  std::shared_ptr<DataBase> top() override { return (*min)->top(); }
};

// A static file iterator.
class JsonSingleStreamIterator : public Iterator<json> {
  long sId;
  std::mutex mu;
  std::queue<JsonElement> content;

  void getLine(json& current, long& currentValue) override {
    std::lock_guard<std::mutex> lock(mu);
    if (content.size() > 0) {
      std::string currline;
      current = content.front().data;
      currentValue = content.front().id;
      content.pop();
    } else {
      throw VnV::VnVExceptionBase("get line called when no line available");
    }
  }

 public:
  JsonSingleStreamIterator(long streamId_) : sId(streamId_) {}

  bool hasNext() override {
    std::lock_guard<std::mutex> lock(mu);
    return content.size() > 0;
  }

  long streamId() const override { return sId; }

  void add(long i, const json& d) {
    std::lock_guard<std::mutex> lock(mu);
    JsonElement j(i, d);
    content.emplace(j);
  }

  ~JsonSingleStreamIterator() {}
};

class JsonPortStreamIterator : public MultiStreamIterator<JsonSingleStreamIterator, json> {
 protected:
  std::string pass;
  std::string uname;
  int port = 0;
  std::mutex mu;
  bool doneMessageRecieved = false;
  std::map<std::string, json> responses;

 public:
  JsonPortStreamIterator(std::string p, const json& config) : MultiStreamIterator<JsonSingleStreamIterator, json>() {
    port = std::atoi(p.c_str());
    pass = config.value("password", "password");
    uname = config.value("username", "root");
  };

  bool authorize(std::string username, std::string password) {
    std::cout << "AUth " << username << " : " << password << " " << uname << " " << pass << "" << password.compare(pass)
              << " " << username.compare(uname) << std::endl;
    return (username.compare(uname) == 0 && password.compare(pass) == 0);
  }

  void done() { doneMessageRecieved = true; }

  virtual bool isDone() override {
    if (!doneMessageRecieved) {
      return false;
    }
    return !hasNext();
  }

  virtual void respond(long id, long jid, const json& response) override {
    // Wait for it to finish reading.
    std::lock_guard<std::mutex> lock(mu);
    std::ostringstream oss;
    oss << id << ":" << jid;
    responses[oss.str()] = json::parse(response.dump());  // Deep copy.]
  }

  virtual bool getResponse(long id, long jid, json& response) {
    std::lock_guard<std::mutex> lock(mu);
    std::ostringstream oss;
    oss << id << ":" << jid;
    std::string key = oss.str();
    auto it = responses.find(key);
    if (it != responses.end()) {
      response = json::parse(it->second.dump());
      responses.erase(it);
      return true;
    }

    return false;
  }

  ~JsonPortStreamIterator() {}
};

template <typename T> T WriteDataJson(IDataType_vec gather) {
  void* outdata;
  T main = T::array();
  for (int i = 0; i < gather.size(); i++) {
    IDataType_ptr d = gather[i];

    T cj = T::array();

    std::map<std::string, PutData> types = d->getLocalPutData();
    for (auto& it : types) {
      T childJson = T::object();

      PutData& p = it.second;
      outdata = d->getPutData(it.first);
      childJson[JSD::node] = JSN::shape;
      childJson[JSD::name] = p.name;
      childJson[JSD::shape] = p.shape;
      int count = std::accumulate(p.shape.begin(), p.shape.end(), 1, std::multiplies<>());

      switch (p.datatype) {
      case SupportedDataType::DOUBLE: {
        double* dd = (double*)outdata;
        if (p.shape.size() > 0) {
          std::vector<double> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::dtype] = JST::Double;

        break;
      }

      case SupportedDataType::LONG: {
        long* dd = (long*)outdata;
        if (p.shape.size() > 0) {
          std::vector<long> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::dtype] = JST::Long;

        break;
      }

      case SupportedDataType::STRING: {
        std::string* dd = (std::string*)outdata;
        if (p.shape.size() > 0) {
          std::vector<std::string> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::dtype] = JST::String;

        break;
      }

      case SupportedDataType::JSON: {
        json* dd = (json*)outdata;
        if (p.shape.size() > 0) {
          std::vector<json> da(dd, dd + count);
          childJson[JSD::value] = da;
        } else {
          childJson[JSD::value] = *dd;
        }
        childJson[JSD::sdt] = JST::Json;

        break;
      }
      }
      cj.push_back(childJson);
    }
    main.push_back(cj);
  }
  return main;
}

template <typename T> class StreamWriter {
 public:
  virtual void initialize(nlohmann::json& config, bool readMode) = 0;
  virtual void finalize(ICommunicator_ptr worldComm, long duration) = 0;
  virtual void newComm(long id, const T& obj, ICommunicator_ptr comm) = 0;
  virtual void write(long id, const T& obj, long jid) = 0;

  virtual bool supportsFetch() { return false; }
  virtual bool fetch(long id, long jid, json& obj) { return false; }

  virtual nlohmann::json getConfigurationSchema(bool readMode) = 0;

  StreamWriter(){};
  virtual ~StreamWriter(){

  };
};

template <typename T> class PortStreamWriter : public StreamWriter<T> {
 
 std::string username;
 std::string password;
 std::string port;
 std::string reader; 

   virtual std::string autostart(const json& config) {
#ifdef WITH_LIBCURL

    std::string readerUrl = config.value("url", "localhost:5000");
    std::string newurl = readerUrl + "/files/autostart";

    std::ostringstream oss;
    oss << "reader=" << reader 
        << "&username=" << username << ""
        << "&password=" << password << ""
        << "&name=" << config.value("name", "vnv_report") << ""
        << "&__token__=" << config.value("vnvpass", "password") << "";
       
    if (config.value("persist", true)) {
      oss << "&persist=on";
    }

    std::string f = oss.str();
    VnV::Curl::CurlWrapper& w = Curl::CurlWrapper::instance();
    w.clearData();
    w.setUrl(newurl);
    w.setPostFields(f.c_str());
    w.send();
    long http_code = w.getCode();
    if (w.getCode() == 200) {
      std::cout << "Auto Start Successfull" << std::endl;
      return w.getData();
    } 
#endif
   throw VnV::VnVExceptionBase("Auto Start Failed");

}

 public:

  std::string init(std::string reader, const json& config) {
    
    username = config.value("username", "root");
    password = config.value("password", "password");
    this->reader = reader;
   
    std::string newFileStub;
    
    if (config.contains("autostart")) {
       newFileStub = autostart(config["autostart"]);
    } else {
       newFileStub = config["filename"].get<std::string>();
    }

    VnV::Curl::CurlWrapper::instance().addBasic(username,password);
    return newFileStub;
  }
  
  std::string getUsername() {
    return username;
  }

  std::string getPassword() {
    return password;
  }
  

  
};

template <typename T> class StreamManager : public OutputEngineManager {
  long vnvTimeStamp = 0;
  long recv = 0;

  long getNextId(ICommunicator_ptr comm, long myVal) {
    if (myVal > vnvTimeStamp) {
      vnvTimeStamp = myVal;
    }
    comm->AllReduce(&id, 1, &recv, sizeof(long), VnV::OpType::MAX);
    vnvTimeStamp = recv + 1;

    return recv;
  }

  static json getWorldRanks(ICommunicator_ptr comm, int root = 0) {
    std::vector<int> res(comm->Rank() == root ? comm->Size() : 0);
    int rank = comm->world()->Rank();
    comm->Gather(&rank, 1, res.data(), sizeof(int), root);
    json nJson = res;
    return nJson;
  }

 protected:
  bool inMemory;
  long id = 0;

  std::set<long> commids;
  std::string outputFile;
  std::shared_ptr<StreamWriter<T>> stream;

  std::string getId() { return std::to_string(StreamManager<T>::id++); }

  virtual void write(T& j) {
    j[JSD::duration] = VnV::RunTime::instance().duration();
    if (stream != nullptr) {
      stream->write(comm->uniqueId(), j, StreamManager<T>::id++);
    }
  }

  virtual void syncId() { StreamManager<T>::id = getNextId(comm, StreamManager<T>::id); }

  virtual void setComm(ICommunicator_ptr comm, bool syncIds) {
    setCommunicator(comm);
    long id = comm->uniqueId();

    if (commids.find(id) == commids.end()) {
      json procList = getWorldRanks(comm);

      if (comm->Rank() == getRoot()) {
        T nJson = T::object();
        nJson[JSD::name] = "comminfo";
        nJson[JSD::node] = JSN::commInfo;
        nJson[JSD::comm] = id;
        nJson[JSD::commList] = procList;

        nJson[JSD::duration] = VnV::RunTime::instance().duration();

        stream->newComm(id, nJson, comm);
      }
      commids.insert(id);
    }

    if (syncIds) {
      syncId();
    }
  }

 public:
  StreamManager(std::shared_ptr<StreamWriter<T>> stream_) : stream(stream_) {}

  virtual ~StreamManager() {}

  virtual bool fetch(long id, const std::string& message, const json& schema, long timeoutInSeconds, json& response,
                     long jid) {
    // Write the schema provided and the expiry time to the file.

    long expiry =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() +
        timeoutInSeconds;

    json j = T::object();
    j["schema"] = schema;
    j["expires"] = expiry;
    j["message"] = message;
    j["jid"] = jid;
    j["id"] = id;
    j[JSD::node] = JSN::fetch;
    write(j);

    if (stream->supportsFetch()) {
      // Loop until timeout
      while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                 .count() < expiry) {
        if (stream->fetch(id, jid, response)) {
          auto js = T::object();
          js[JSD::node] = JSN::fetchSuccess;
          write(js);
          return true;
        }

        // Go to sleep for a second to avoid spamming the filesystem
        sleep(1);
      }
    }

    // We timed out so continue.
    auto js = T::object();
    js[JSD::node] = JSN::fetchFail;
    write(j);

    return false;
  }

  bool Fetch(std::string message, const json& schema, long timeoutInSeconds, json& response) override {
    std::string line = response.dump();
    long size = -1;

    if (comm->Rank() == getRoot()) {
      response = json::object();
      bool worked = fetch(comm->uniqueId(), message, schema, timeoutInSeconds, response, StreamManager<T>::id++);
      line = response.dump();
      if (worked) {
        size = line.size();
      }
    }
    comm->BroadCast(&size, 1, sizeof(std::size_t), getRoot());

    if (size < 0) {
      return false;
    }

    line.resize(size);
    comm->BroadCast(const_cast<char*>(line.data()), size, sizeof(char), getRoot());
    response = json::parse(line);
    return true;
  }

#define LTypes      \
  X(double, Double) \
  X(long long, Long) X(bool, Bool) X(std::string, String) X(json, Json)
#define X(typea, typeb)                                                                \
  void Put(std::string variableName, const typea& value, const MetaData& m) override { \
    if (comm->Rank() == getRoot()) {                                                   \
      T j = T::object();                                                               \
      j[JSD::name] = variableName;                                                     \
      j[JSD::dtype] = JST::typeb;                                                      \
      j[JSD::value] = value;                                                           \
      j[JSD::shape] = json::object();                                                  \
      j[JSD::node] = JSN::shape;                                                       \
      j[JSD::meta] = m;                                                                \
      write(j);                                                                        \
    }                                                                                  \
  }
  LTypes
#undef X
#undef LTypes

      void
      Put(std::string variableName, IDataType_ptr data, const MetaData& m) {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::dataTypeStarted;
      j[JSD::name] = variableName;
      j[JSD::meta] = m;
      j[JSD::dtype] = data->getKey();
      write(j);
    }

    data->Put(this);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::dataTypeEnded;
      write(j);
    }
  }

  void PutGlobalArray(long long dtype, std::string variableName, IDataType_vec data, std::vector<int> gsizes,
                      std::vector<int> sizes, std::vector<int> offset, const MetaData& m) {
    VnV::DataTypeCommunication d(comm);

    // Gather all on the root processor
    IDataType_vec gather = d.GatherV(data, dtype, getRoot(), gsizes, sizes, offset, false);

    if (gather.size() > 0 && comm->Rank() == getRoot()) {
      json j;
      j[JSD::name] = variableName;
      j[JSD::node] = JSN::shape;
      j[JSD::shape] = gsizes;
      j[JSD::dtype] = JST::GlobalArray;
      j[JSD::key] = dtype;
      j[JSD::meta] = m;
      j[JSD::children] = WriteDataJson<json>(gather);
      write(j);
    }
  }

  void Log(ICommunicator_ptr logcomm, const char* package, int stage, std::string level, std::string message) {
    // Save the current communicator
    ICommunicator_ptr commsave = this->comm;

    // Set the logcomm as the current communication
    setComm(logcomm, true);

    // Write the log
    if (comm->Rank() == getRoot()) {
      T log = T::object();
      log[JSD::package] = package;
      log[JSD::stage] = stage;
      log[JSD::level] = level;
      log[JSD::message] = message;
      log[JSD::node] = JSN::log;
      log[JSD::name] = std::to_string(StreamManager<json>::id++);
      log[JSD::comm] = comm->uniqueId();
      write(log);
    }

    // Set the old comm
    if (commsave != nullptr) {
      setComm(commsave, false);
    }
  }

  json getConfigurationSchema(bool readMode) override { return stream->getConfigurationSchema(readMode); }

  void finalize(ICommunicator_ptr worldComm, long duration) override {
    setComm(worldComm, false);
    if (comm->Rank() == getRoot()) {
      T nJson = T::object();
      nJson[JSD::node] = JSN::duration;
      nJson[JSD::duration] = duration;
      write(nJson);
    }

    stream->finalize(worldComm, duration);
  }

  T getNodeMap(ICommunicator_ptr world) {
    std::string name = world->ProcessorName();
    auto h = StringUtils::simpleHash(name);
    std::vector<long long> results(world->Rank() == getRoot() ? world->Size() : 0);
    world->Gather(&h, 1, results.data(), sizeof(long long), 0);
    if (world->Rank() == getRoot()) {
      T j = T::object();
      for (int i = 0; i < results.size(); i++) {
        std::string proc = std::to_string(results[i]);
        if (j.contains(proc)) {
          j[proc].push_back(i);
        } else {
          j[proc] = {i};
        }
      }
      return j;
    }
    return T::object();
  }

  void setFromJson(ICommunicator_ptr worldComm, json& config, bool readMode) override {
    if (!readMode) {
      stream->initialize(config, readMode);
      setComm(worldComm, false);

      T node_map = getNodeMap(worldComm);

      if (comm->Rank() == getRoot()) {
        // This is the first one so we send over the info. .
        T nJson = T::object();
        nJson[JSD::title] = "VnV Simulation Report";
        nJson[JSD::date] =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        nJson[JSD::name] = "MainInfo";
        nJson[JSD::node] = JSN::info;
        nJson[JSD::worldSize] = comm->Size();
        nJson[JSD::spec] = RunTime::instance().getFullJson();
        nJson[JSD::nodeMap] = node_map;
        nJson[JSD::mpiversion] = comm->VersionLibrary();
        nJson[JSD::prov] = RunTime::instance().getProv().toJson();

        write(nJson);
      }
    }
  }

  void file(ICommunicator_ptr comm, std::string packageName, std::string name, bool inputFile, std::string filename,
            std::string reader) {
    setComm(comm, false);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::file;
      j[JSD::input] = inputFile;

      ProvFile pf(filename, reader);
      pf.comm = comm->uniqueId();
      pf.package = packageName;
      pf.name = name;

      j[JSD::prov] = pf.toJson();
      write(j);
    }
  }

  void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageVal) override {
    syncId();
    if (comm->Rank() == getRoot()) {
      T j = T::object();

      j[JSD::node] = (type == InjectionPointType::End || type == InjectionPointType::Single)
                         ? JSN::injectionPointEnded
                         : JSN::injectionPointIterEnded;
      write(j);
    }
  }

  void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string id,
                                     InjectionPointType type, std::string stageVal, std::string filename,
                                     int line) override {
    setComm(comm, true);
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::name] = id;
      j[JSD::package] = packageName;
      j[JSD::comm] = comm->uniqueId();
      j[JSD::filename] = filename;
      j[JSD::line] = line;
      j[JSD::stageId] = stageVal;

      if (type == InjectionPointType::Begin || type == InjectionPointType::Single) {
        j[JSD::node] = JSN::injectionPointStarted;
      } else {
        j[JSD::node] = JSN::injectionPointIterStarted;
      }
      write(j);
    }
  }
  void testStartedCallBack(std::string packageName, std::string testName, bool internal, long uuid) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::name] = testName;
      j[JSD::package] = packageName;
      j[JSD::internal] = internal;
      j[JSD::node] = JSN::testStarted;
      j[JSD::testuid] = uuid;
      write(j);
    }
  }

  void testFinishedCallBack(bool result_) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::testFinished;
      j[JSD::result] = result_;

      write(j);
    }
  }

  void actionStartedCallBack(ICommunicator_ptr comm, std::string package, std::string name,
                             ActionStage::type stage) override {
    setComm(comm, true);
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::actionStarted;
      j[JSD::name] = name;
      j[JSD::package] = package;
      j[JSD::stage] = stage;
      write(j);
    }
  };

  virtual void actionEndedCallBack(ActionStage::type stage) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::actionFinished;
      j[JSD::stage] = stage;
      write(j);
    }
  };

  void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string unitTestName) override {
    setComm(comm, true);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::unitTestStarted;
      j[JSD::name] = unitTestName;
      j[JSD::package] = packageName;
      write(j);
    }
  }

  virtual void packageOptionsStartedCallBack(ICommunicator_ptr comm, std::string packageName) override {
    setComm(comm, true);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::packageOptionsStarted;
      j[JSD::package] = packageName;
      write(j);
    }
  }

  virtual void packageOptionsEndedCallBack(std::string packageName) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::packageOptionsFinished;
      write(j);
    }
  }

  void unitTestFinishedCallBack(IUnitTest* tester) {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::unitTestFinished;
      T c = T::array();
      for (auto it : tester->getResults()) {
        T kk = T::object();
        kk[JSD::name] = std::get<0>(it);
        kk[JSD::description] = std::get<1>(it);
        kk[JSD::result] = std::get<2>(it);
        c.push_back(kk);
      }
      j[JSD::results] = c;
      write(j);
    }
  }

  // IInternalOutputEngine interface
  std::string print() override { return "SS"; }
};

constexpr const char* extension = ".fs";

template <typename T, typename V> class FileStream : public StreamWriter<V> {
 protected:
  std::string filestub;
  virtual ~FileStream() {}

  std::string getFileName(long id, bool makedir = true) {
    std::vector<std::string> fname = {std::to_string(id)};
    return getFileName_(filestub, {std::to_string(id) + extension}, true);
  }

  std::string getFileName_(std::string root, std::vector<std::string> fname, bool mkdir) {
    fname.insert(fname.begin(), root);
    std::string filename = fname.back();
    fname.pop_back();
    std::string fullname = VnV::DistUtils::join(fname, 0777, mkdir);
    return VnV::DistUtils::join({fullname, filename}, 0777, false);
  }

 public:
  virtual void initialize(json& config, bool readMode) override {
    if (!readMode) {
      this->filestub = config["filename"].template get<std::string>();
    }
  }

  virtual nlohmann::json getConfigurationSchema(bool readMode) override {
    return readMode ? R"({})"_json : R"(
      { "type" : "object" , 
        "properties" : {
           "filename" : {"type":"string"}
        },
        "required" : ["filename"] 
      }
    )"_json;
  };

  virtual void finalize(ICommunicator_ptr wcomm, long duration) = 0;

  virtual void newComm(long id, const V& obj, ICommunicator_ptr comm) = 0;

  virtual void write(long id, const V& obj, long jid) = 0;
};

template <class DB> class StreamParserTemplate {
 public:
  template <class T> class ParserVisitor : public VisitorLock {
   public:
    std::shared_ptr<Iterator<T>> jstream;
    typename DB::RootNode* _rootInternal;

    typename DB::RootNode* rootInternal() { return _rootInternal; }

    virtual std::pair<long, std::set<long>> visitCommNode(const T& j) {
      std::pair<long, std::set<long>> res;

      for (auto it : j[JSD::commList].items()) {
        res.second.insert(it.value().template get<long>());
      }

      res.first = j[JSD::comm].template get<long>();

      return res;
    }

    template <typename A> std::shared_ptr<A> mks_str(std::string noName = "") {
      std::shared_ptr<A> base_ptr;
      base_ptr.reset(new A());
      rootInternal()->registerNode(base_ptr);
      base_ptr->setname(noName);
      base_ptr->setstreamId(jstream->streamId());
      return base_ptr;
    }

    template <typename A> std::shared_ptr<A> mks(const T& j) {
      auto a = mks_str<A>(j[JSD::name].template get<std::string>());
      if (j.contains(JSD::meta)) {
        MetaDataWrapper m;
        for (auto it : j[JSD::meta].items()) {
          m.add(it.key(), it.value().template get<std::string>());
        }
        a->setmetadata(m);
      }
      return a;
    }

    template <typename A, typename V> std::shared_ptr<A> visitShapeNode(const T& j) {
      std::shared_ptr<A> n = mks<A>(j);
      std::size_t shapeSize = j[JSD::shape].size();

      std::vector<std::size_t> shape;
      std::vector<V> value;

      shape.reserve(shapeSize);
      for (auto it : j[JSD::shape].items()) {
        shape.push_back(it.value().template get<std::size_t>());
      }

      // Set the scalar value.
      if (shapeSize == 0) {
        value.reserve(1);
        value.push_back(j[JSD::value].template get<V>());
      } else {
        value.reserve(shapeSize);
        for (auto it : j[JSD::value].items()) {
          value.push_back(it.value().template get<V>());
        }
      }

      n->setShape(std::move(shape));
      n->setValue(std::move(value));

      return n;
    }

    virtual std::shared_ptr<typename DB::StringNode> visitStringNode(const T& j) {
      return visitShapeNode<typename DB::StringNode, std::string>(j);
    }

    virtual std::shared_ptr<typename DB::DoubleNode> visitDoubleNode(const T& j) {
      return visitShapeNode<typename DB::DoubleNode, double>(j);
    }
    virtual std::shared_ptr<typename DB::BoolNode> visitBoolNode(const T& j) {
      return visitShapeNode<typename DB::BoolNode, bool>(j);
    };
    virtual std::shared_ptr<typename DB::LongNode> visitLongNode(const T& j) {
      return visitShapeNode<typename DB::LongNode, long>(j);
    }

    virtual std::shared_ptr<typename DB::JsonNode> visitJsonNode(const T& j) {
      auto n = mks<typename DB::JsonNode>(j);
      n->open(true);

      std::vector<std::size_t> shape;
      std::vector<std::string> value;

      std::size_t shapeSize = j[JSD::shape].size();
      shape.reserve(shapeSize);

      for (auto it : j[JSD::shape].items()) {
        shape.push_back(it.value().template get<std::size_t>());
      }
      if (shapeSize == 0) {
        value.push_back(j[JSD::value].dump());
      } else {
        value.reserve(j[JSD::value].size());
        for (auto it : j[JSD::value].items()) {
          value.push_back(it.value().dump());
        }
      }
      n->setShape(std::move(shape));
      n->setValue(std::move(value));
      n->open(false);
      return n;
    }

    virtual std::shared_ptr<DataBase> visitGlobalArrayNode(const T& j) {
      auto n = mks<typename DB::ShapeNode>(j);
      n->open(true);
      // Load the shape.

      std::vector<std::size_t> shape;
      std::vector<std::shared_ptr<DataBase>> value;

      for (auto it : j[JSD::shape].items()) {
        shape.push_back(it.value().template get<std::size_t>());
      }
      long long key = j[JSD::key].template get<long long>();

      int count = 0;
      value.reserve(j[JSD::children].size());
      for (auto& it : j[JSD::children].items()) {
        auto elm = mks_str<typename DB::DataNode>(std::to_string(count++));
        elm->setlocal(true);
        elm->setkey(key);

        // Push elm to the top of this stream.
        jstream->push(elm);
        int count1 = 0;
        for (auto& itt : it.value().items()) {
          visit(itt.value(), count1++);
        }
        jstream->pop();
        value.push_back(elm);
      }

      n->setShape(std::move(shape));
      n->setValue(std::move(value));

      n->open(false);
      return n;
    }

    virtual std::shared_ptr<typename DB::InfoNode> visitInfoNode(const T& j) {
      auto n = mks<typename DB::InfoNode>(j);
      n->open(true);
      n->settitle(j[JSD::title].template get<std::string>());
      n->setdate(j[JSD::date].template get<long>());
      n->setprov(std::make_shared<VnVProv>(j[JSD::prov]));

      rootInternal()->setspec(j[JSD::spec]);
      auto cim = std::dynamic_pointer_cast<typename DB::CommInfoNode>(rootInternal()->getCommInfoNode());
      cim->setworldSize(j[JSD::worldSize].template get<int>());
      cim->setnodeMap(j[JSD::nodeMap]);
      cim->setversion(j[JSD::mpiversion].template get<std::string>());
      n->open(false);
      return n;
    };

    virtual void visitFileNode(const T& j) {
      std::shared_ptr<ProvFile> pf = std::make_shared<ProvFile>(j[JSD::prov]);
      bool input = j[JSD::input].template get<bool>();

      if (input) {
        rootInternal()->getInfoNode()->addInputFile(pf);
      } else {
        rootInternal()->getInfoNode()->addOutputFile(pf);
      }
    }

    virtual std::shared_ptr<typename DB::LogNode> visitLogNode(const T& j) {
      auto n = mks<typename DB::LogNode>(j);
      n->open(true);
      n->setpackage(j[JSD::package].template get<std::string>());
      n->setlevel(j[JSD::level].template get<std::string>());
      n->setmessage(j[JSD::message].template get<std::string>());
      n->setstage(std::to_string(j[JSD::stage].template get<int>()));
      n->setcomm(std::to_string(j[JSD::comm].template get<long>()));
      n->open(false);

      return n;
    };

    virtual std::shared_ptr<typename DB::TestNode> visitActionNodeStarted(const T& j) {
      std::string name = j[JSD::name].template get<std::string>();
      std::string package = j[JSD::package].template get<std::string>();
      std::string pn = package + ":" + name;

      if (!rootInternal()->getActions()->contains(pn)) {
        auto n = mks<typename DB::TestNode>(j);
        n->setpackage(package);
        n->open(true);
        n->setvalue(rootInternal()->getVnVSpec().action(n->getpackage(), n->getName()));
        rootInternal()->getActions()->insert(pn, n);
        return n;
      }
      return std::dynamic_pointer_cast<typename DB::TestNode>(rootInternal()->getActions()->get(pn)->get(0));
    };

    virtual std::shared_ptr<typename DB::TestNode> visitActionNodeEnded(const T& j, std::shared_ptr<DataBase> node) {
      auto n = std::dynamic_pointer_cast<typename DB::TestNode>(node);
      ActionStage::type stage = j[JSD::stage].template get<ActionStage::type>();
      if (stage == ActionStage::final) {
        n->open(false);
      }
      return n;
    };

    virtual std::shared_ptr<typename DB::TestNode> visitPackageNodeStarted(const T& j) {
      auto n = mks_str<typename DB::TestNode>("Information");
      n->open(true);
      n->setpackage(j[JSD::package].template get<std::string>());
      n->setvalue(rootInternal()->getVnVSpec().package(n->getpackage()));
      return n;
    };

    virtual std::shared_ptr<typename DB::TestNode> visitPackageNodeEnded(const T& j, std::shared_ptr<DataBase> node) {
      auto n = std::dynamic_pointer_cast<typename DB::TestNode>(node);
      n->open(false);
      return n;
    };

    virtual std::shared_ptr<typename DB::UnitTestNode> visitUnitTestNodeStarted(const T& j) {
      auto n = mks<typename DB::UnitTestNode>(j);
      n->open(true);
      n->setpackage(j[JSD::package].template get<std::string>());

      json templs = rootInternal()->getVnVSpec().unitTest(n->getpackage(), n->getName());
      n->setvalue(templs["docs"].get<std::string>());

      std::map<std::string, std::string> testTemplate;
      for (auto it : templs["tests"].items()) {
        testTemplate[it.key()] = it.value().template get<std::string>();
      }
      n->settestTemplate(std::move(testTemplate));

      return n;
    };

    virtual std::shared_ptr<typename DB::UnitTestNode> visitUnitTestNodeEnded(const T& j,
                                                                              std::shared_ptr<DataBase> node) {
      auto n = std::dynamic_pointer_cast<typename DB::UnitTestNode>(node);

      auto results = std::dynamic_pointer_cast<typename DB::UnitTestResultsNode>(n->getResults());

      for (auto& it : j[JSD::results].items()) {
        auto r = mks<typename DB::UnitTestResultNode>(it.value());
        r->setdesc(it.value()[JSD::description].template get<std::string>());
        r->setresult(it.value()[JSD::result].template get<bool>());
        results->insert(r->getname(), r);
      }
      n->open(false);
      return n;
    }

    virtual std::shared_ptr<DataBase> visitDataNodeStarted(const T& j) {
      auto n = mks<typename DB::DataNode>(j);
      n->open(true);
      n->setkey(j[JSD::dtype].template get<long long>());
      return n;
    }

    virtual std::shared_ptr<IDataNode> visitDataNodeEnded(const T& j, std::shared_ptr<DataBase> node) {
      node->open(false);
      return std::dynamic_pointer_cast<typename DB::DataNode>(node);
    }

    virtual std::shared_ptr<typename DB::TestNode> visitTestNodeStarted(const T& j) {
      auto n = mks<typename DB::TestNode>(j);
      n->open(true);
      n->setuid(j[JSD::testuid].template get<long>());
      n->setpackage(j[JSD::package].template get<std::string>());
      n->setvalue((j[JSD::internal].template get<bool>())
                      ? ""
                      : rootInternal()->getVnVSpec().test(n->getpackage(), n->getName()));
      n->setinternal(j[JSD::internal]);
      return n;
    }

    virtual std::shared_ptr<typename DB::TestNode> visitTestNodeIterStarted(
        const T& j, std::shared_ptr<typename DB::TestNode> node) {
      return node;
    };

    virtual std::shared_ptr<typename DB::TestNode> visitTestNodeIterEnded(const T& j, std::shared_ptr<DataBase> node) {
      return std::dynamic_pointer_cast<typename DB::TestNode>(node);
    };

    virtual std::shared_ptr<typename DB::TestNode> visitTestNodeEnded(const T& j, std::shared_ptr<DataBase> node) {
      auto n = std::dynamic_pointer_cast<typename DB::TestNode>(node);
      n->setresult(j[JSD::result].template get<bool>());
      return n;
    };

    void addSource(const json& j, std::shared_ptr<typename DB::InjectionPointNode> node) {
      if (j.contains(JSD::filename)) {
        std::string fname = j[JSD::filename].template get<std::string>();
        long line = j[JSD::line].template get<long>();
        std::string stage = j[JSD::stageId].template get<std::string>();
        node->addToSourceMap(stage, fname, line);
      }
    }

    virtual std::shared_ptr<typename DB::InjectionPointNode> visitInjectionPointStartedNode(const T& j,
                                                                                            long elementId) {
      auto n = mks<typename DB::InjectionPointNode>(j);
      n->setpackage(j[JSD::package].template get<std::string>());
      n->setvalue(rootInternal()->getVnVSpec().injectionPoint(n->getpackage(), n->getName()));
      n->setcommId(j[JSD::comm].template get<long>());
      n->setstartIndex(elementId);
      n->open(true);
      addSource(j, n);

      return n;
    }

    virtual std::shared_ptr<typename DB::TestNode> visitFetchNode(const T& j,
                                                                  std::shared_ptr<typename DB::TestNode> node,
                                                                  long elementId) {
      std::string schema = j["schema"].dump();
      long expiry = j["expires"].template get<long>();
      long id = j["id"].template get<long>();
      long jid = j["jid"].template get<long>();
      std::string message = j["message"].template get<std::string>();

      node->setFetchRequest(schema, id, jid, expiry, message);
      return node;
    }

    virtual std::shared_ptr<typename DB::TestNode> visitFetchFailedNode(const T& j,
                                                                        std::shared_ptr<typename DB::TestNode> node,
                                                                        long elementId) {
      node->resetFetchRequest();
      return node;
    }
    virtual std::shared_ptr<typename DB::TestNode> visitFetchSuccessNode(const T& j,
                                                                         std::shared_ptr<typename DB::TestNode> node,
                                                                         long elementId) {
      node->resetFetchRequest();
      return node;
    }

    virtual std::shared_ptr<typename DB::InjectionPointNode> visitInjectionPointEndedNode(
        const T& j, std::shared_ptr<DataBase> node, long elementId) {
      auto n = std::dynamic_pointer_cast<typename DB::InjectionPointNode>(node);
      n->setendIndex(elementId);
      n->open(false);
      return n;
    }

    virtual std::shared_ptr<typename DB::InjectionPointNode> visitInjectionPointIterStartedNode(
        const T& j, std::shared_ptr<DataBase> node, long elementId) {
      auto n = std::dynamic_pointer_cast<typename DB::InjectionPointNode>(node);
      addSource(j, n);
      return n;
    };

    virtual std::shared_ptr<typename DB::InjectionPointNode> visitInjectionPointIterEndedNode(
        const T& j, std::shared_ptr<DataBase> node, long elementId) {
      auto n = std::dynamic_pointer_cast<typename DB::InjectionPointNode>(node);
      return n;
    }

    ParserVisitor() {}

    virtual void set(std::shared_ptr<Iterator<T>> jstream_, typename DB::RootNode* rootNode) {
      jstream = jstream_;
      _rootInternal = rootNode;
      rootInternal()->setVisitorLock(this);
    }

    virtual ~ParserVisitor() { rootInternal()->setVisitorLock(nullptr); }

    std::atomic_bool read_lock = ATOMIC_VAR_INIT(false);
    std::atomic_bool write_lock = ATOMIC_VAR_INIT(false);

    virtual void process() {
      jstream->start_stream_reader();
      long i = 0;
      bool changed = false;

      while (!jstream->isDone()) {
        i = ++i % 1000;
        if (changed && i == 999) {
          rootInternal()->persist();
          changed = false;
        }

        if (jstream->hasNext()) {
          setWriteLock();
          try {
            auto p = jstream->next();
            visit(p.first, p.second);
          } catch (std::exception e) {
            throw VnV::VnVExceptionBase(e.what());
          }

          releaseWriteLock();
          changed = true;
        }
      }
      rootInternal()->persist();
      rootInternal()->setProcessing(false);
      jstream->stop_stream_reader();
    }

    virtual void setWriteLock() {
      while (read_lock.load(std::memory_order_relaxed)) {
      }
      write_lock.store(true, std::memory_order_relaxed);
    }

    virtual void releaseWriteLock() { write_lock.store(false, std::memory_order_relaxed); }

    void lock() override {
      read_lock.store(true, std::memory_order_relaxed);
      while (write_lock.load(std::memory_order_relaxed)) {
      }
    }

    void release() override { read_lock.store(false, std::memory_order_relaxed); }

    void childNodeDispatcher(std::shared_ptr<DataBase> child, long elementId, long duration) {
      std::shared_ptr<DataBase> parent = jstream->top();

      DataBase::DataType ptype = (parent == nullptr) ? DataBase::DataType::Root : parent->getType();
      DataBase::DataType ctype = child->getType();

      if (ptype == DataBase::DataType::Shape) {
        auto p = std::dynamic_pointer_cast<typename DB::ShapeNode>(parent);
        p->add(child);

      } else if (ptype == DataBase::DataType::Test) {
        std::shared_ptr<ITestNode> p1 = std::dynamic_pointer_cast<ITestNode>(parent);
        if (ctype == DataBase::DataType::Log) {
          p1->getLogs()->add(child);
        } else {
          p1->getData()->insert(child->getName(), child);
        }

      } else if (ptype == DataBase::DataType::UnitTest) {
        auto p = std::dynamic_pointer_cast<typename DB::UnitTestNode>(parent);
        if (ctype == DataBase::DataType::Log) {
          p->getLogs()->add(child);
        } else {
          p->getData()->insert(child->getName(), child);
        }

      } else if (ptype == DataBase::DataType::Data) {
        auto p = std::dynamic_pointer_cast<typename DB::DataNode>(parent);
        if (ctype == DataBase::DataType::Log) {
          p->getLogs()->add(child);
        } else {
          p->getData()->insert(child->getName(), child);
        }

      } else if (ptype == DataBase::DataType::InjectionPoint &&
                 std::dynamic_pointer_cast<typename DB::InjectionPointNode>(parent)->getisOpen()) {
        auto p = std::dynamic_pointer_cast<typename DB::InjectionPointNode>(parent);

        if (ctype == DataBase::DataType::Test) {
          if (child->getAsTestNode(child)->isInternal()) {
            p->setinternal(std::dynamic_pointer_cast<typename DB::TestNode>(child));
          } else {
            p->getTests()->add(child);
          }

        } else if (ctype == DataBase::DataType::Log) {
          p->getLogs()->add(child);
          rootInternal()->addIDN(child->getId(), child->getStreamId(), node_type::LOG, elementId, duration, "Log");
        } else {
          throw VnVExceptionBase("Hmmmm");
        }

      } else if (ctype == DataBase::DataType::InjectionPoint) {
      } else if (ctype == DataBase::DataType::Log) {
        rootInternal()->addIDN(child->getId(), child->getStreamId(), node_type::LOG, elementId, duration, "Log");
      } else {
        throw VnVExceptionBase("Unsupported Parent element type");
      }
    }

    void visit(const T& j, long elementId) {
      std::string node = j[JSD::node].template get<std::string>();

      long duration = j[JSD::duration].template get<long>();

      if (node == JSN::duration) {
        rootInternal()->setduration(j[JSD::duration].template get<long>());

      } else if (node == JSN::done) {
        // DONE
      } else if (node == JSN::commInfo) {
        auto p = visitCommNode(j);
        rootInternal()->getCommInfoNode()->add(p.first, p.second);

      } else if (node == JSN::file) {
        visitFileNode(j);
      } else if (node == JSN::dataTypeEnded) {
        visitDataNodeEnded(j, jstream->pop());

      } else if (node == JSN::dataTypeStarted) {
        auto n = visitDataNodeStarted(j);
        childNodeDispatcher(n, elementId, duration);
        jstream->push(n);

      } else if (node == JSN::info) {
        rootInternal()->setinfoNode(visitInfoNode(j));
      } else if (node == JSN::fetch) {
        if (jstream->top()->check(DataBase::DataType::Test)) {
          std::shared_ptr<typename DB::TestNode> p = std::dynamic_pointer_cast<typename DB::TestNode>(jstream->top());
          visitFetchNode(j, p, elementId);
        }
      } else if (node == JSN::fetchFail) {
        if (jstream->top()->check(DataBase::DataType::Test)) {
          std::shared_ptr<typename DB::TestNode> p = std::dynamic_pointer_cast<typename DB::TestNode>(jstream->top());
          visitFetchFailedNode(j, p, elementId);
        }
      } else if (node == JSN::fetchSuccess) {
        if (jstream->top()->check(DataBase::DataType::Test)) {
          std::shared_ptr<typename DB::TestNode> p = std::dynamic_pointer_cast<typename DB::TestNode>(jstream->top());
          visitFetchSuccessNode(j, p, elementId);
        }
      } else if (node == JSN::injectionPointEnded) {
        // This injection point is done.
        std::shared_ptr<typename DB::InjectionPointNode> p =
            std::dynamic_pointer_cast<typename DB::InjectionPointNode>(jstream->pop());
        p->setisOpen(false);
        visitInjectionPointEndedNode(j, p, elementId);

        // Close the tests
        auto tests = p->getTests();
        for (int i = 0; i < tests->size(); i++) {
          tests->get(i)->open(false);
        }

        rootInternal()->addIDN(p->getId(), p->getstreamId(), node_type::END, elementId, duration, "End");

      } else if (node == JSN::injectionPointStarted) {
        std::shared_ptr<typename DB::InjectionPointNode> p = visitInjectionPointStartedNode(j, elementId);

        p->setisOpen(true);
        childNodeDispatcher(p, elementId, duration);
        jstream->push(p);

        rootInternal()->addIDN(p->getId(), p->getstreamId(), node_type::START, elementId, duration, "Begin");

      } else if (node == JSN::injectionPointIterEnded) {
        std::shared_ptr<typename DB::InjectionPointNode> p =
            std::dynamic_pointer_cast<typename DB::InjectionPointNode>(jstream->top());
        visitInjectionPointIterEndedNode(j, p, elementId);
        p->setisOpen(false);
        p->setisIter(false);

      } else if (node == JSN::injectionPointIterStarted) {
        assert(jstream->top()->getType() == DataBase::DataType::InjectionPoint);
        std::shared_ptr<typename DB::InjectionPointNode> p =
            std::dynamic_pointer_cast<typename DB::InjectionPointNode>(jstream->top());

        visitInjectionPointIterStartedNode(j, p, elementId);
        std::string stage = j[JSD::stageId].template get<std::string>();
        p->setisOpen(true);
        p->setisIter(true);
        rootInternal()->addIDN(p->getId(), p->getstreamId(), node_type::ITER, elementId, duration, stage);

      } else if (node == JSN::log) {
        std::shared_ptr<typename DB::LogNode> n = visitLogNode(j);
        n->setidentity(elementId);
        childNodeDispatcher(n, elementId, duration);

      } else if (node == JSN::testFinished) {
        auto p = jstream->pop();

        assert(jstream->top()->getType() == DataBase::DataType::InjectionPoint);
        auto pp = std::dynamic_pointer_cast<typename DB::InjectionPointNode>(jstream->top());
        if (pp->getisIter()) {
          visitTestNodeIterEnded(j, p);
        } else {
          visitTestNodeEnded(j, p);
        }

      } else if (node == JSN::testStarted) {
        auto p = jstream->top();
        if (p->getType() != DataBase::DataType::InjectionPoint) {
          throw VnVExceptionBase("Bad Heirrarchy");
        }

        std::shared_ptr<typename DB::InjectionPointNode> pp =
            std::dynamic_pointer_cast<typename DB::InjectionPointNode>(p);

        if (pp->getisIter()) {
          long uid = j[JSD::testuid].template get<long>();
          auto tests = pp->getTests();

          if (j[JSD::internal].template get<bool>()) {
            auto t = std::dynamic_pointer_cast<typename DB::TestNode>(pp->getinternal());
            if (t->getuid() != uid) {
              throw VnV::VnVExceptionBase("Wrong uid for internal test");
            }
            visitTestNodeIterStarted(j, t);
            jstream->push(t);
          } else {
            bool found = false;
            auto t = pp->getTestByUID(uid);
            if (t != nullptr) {
              visitTestNodeIterStarted(j, t);
              jstream->push(t);
            } else {
              throw VnV::VnVExceptionBase("ERROR COULD NOT FIND TEST DURING ITERATION");
            }
          }
        } else {
          std::shared_ptr<typename DB::TestNode> t = visitTestNodeStarted(j);
          childNodeDispatcher(t, elementId, duration);
          jstream->push(t);
        }

      } else if (node == JSN::unitTestFinished) {
        visitUnitTestNodeEnded(j, jstream->pop());

      } else if (node == JSN::unitTestStarted) {
        auto u = visitUnitTestNodeStarted(j);
        rootInternal()->getUnitTests()->add(u);
        jstream->push(u);
      } else if (node == JSN::actionStarted) {
        auto po = visitActionNodeStarted(j);
        jstream->push(po);
      } else if (node == JSN::actionFinished) {
        auto u = visitActionNodeEnded(j, jstream->pop());
      } else if (node == JSN::packageOptionsStarted) {
        auto po = visitPackageNodeStarted(j);
        rootInternal()->getPackages()->insert(po->getPackage(), po);

        jstream->push(po);
      } else if (node == JSN::packageOptionsFinished) {
        auto u = visitPackageNodeEnded(j, jstream->pop());
      } else if (node == JSN::shape) {
        std::string type = j[JSD::dtype].template get<std::string>();

        if (type == JST::String) {
          childNodeDispatcher(visitShapeNode<typename DB::StringNode, std::string>(j), elementId, duration);

        } else if (type == JST::Bool) {
          childNodeDispatcher(visitShapeNode<typename DB::BoolNode, bool>(j), elementId, duration);

        } else if (type == JST::Long) {
          childNodeDispatcher(visitShapeNode<typename DB::LongNode, long>(j), elementId, duration);

        } else if (type == JST::Double) {
          childNodeDispatcher(visitShapeNode<typename DB::DoubleNode, double>(j), elementId, duration);

        } else if (type == JST::Json) {
          childNodeDispatcher(visitJsonNode(j), elementId, duration);

        } else if (type == JST::GlobalArray) {
          childNodeDispatcher(visitGlobalArrayNode(j), elementId, duration);

        } else {
          throw VnVExceptionBase("Bad data type");
        }

      } else {
        throw VnVExceptionBase("Unrecognized Node Type");
      }
    }
  };

  template <class T> class NoLockParserVisitor : public ParserVisitor<T> {
   public:
    virtual void setWriteLock() override {}
    virtual void releaseWriteLock() override {}
  };

  template <typename T, typename V> class RootNodeWithThread : public DB::RootNode {
   public:
    std::shared_ptr<ParserVisitor<V>> visitor;
    std::shared_ptr<T> stream;

    std::thread tworker;
    void run(bool async) {
      if (async) {
        tworker = std::thread(&ParserVisitor<V>::process, visitor.get());
      } else {
        visitor->process();
      }
    }

    virtual void respond(long id, long jid, const std::string& response) override {
      if (stream != nullptr) {
        stream->respond(id, jid, json::parse(response));
      }
    }

    virtual std::thread* getThread() override { return &tworker; }

    static std::shared_ptr<IRootNode> parse(bool async, std::shared_ptr<T> stream,
                                            std::shared_ptr<ParserVisitor<V>> visitor = nullptr) {
      std::shared_ptr<RootNodeWithThread> root = std::make_shared<RootNodeWithThread>();
      root->registerNode(root);
      root->setname("Root Node");

      root->stream = stream;
      root->visitor = (visitor == nullptr) ? std::make_shared<ParserVisitor<V>>() : visitor;

      root->visitor->set(stream, root.get());
      root->open(true);
      root->run(async);
      return root;
    }
  };
};

}  // namespace Nodes
}  // namespace VnV

#endif
