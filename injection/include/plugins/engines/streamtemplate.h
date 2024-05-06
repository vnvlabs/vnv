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
#include <filesystem>

#include "shared/DistUtils.h"
#include "shared/constants.h"
#include "shared/Provenance.h"
#include "shared/exceptions.h"
#include "interfaces/IOutputEngine.h"

namespace VnV {
namespace Nodes {

#define STREAM_IS_CLOSED -199

using namespace VnV::Constants;


template <typename T> T WriteDataJson(IDataType_vec gather) {
  void* outdata;
  T main = T::array();
  for (int i = 0; i < gather.size(); i++) {
    IDataType_ptr d = gather[i];
    T cj = T::array();
    if (d != nullptr) {
      std::map<std::string, PutData> types = d->getLocalPutData();
      for (auto& it : types) {
        T childJson = T::object();

        PutData& p = it.second;
        try {
          outdata = d->getPutData(it.first);  // TODO_THROWS
        } catch (VnV::VnVExceptionBase& e) {
          HTHROW INJECTION_EXCEPTION(
              "DataType is not configured Correctly %s is listed as a type in"
              "get local put data, but getPutData throws when it is passed to it.",
              it.first.c_str());
        }
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
    }
    main.push_back(cj);
  }
  return main;
}

template <typename T> class StreamWriter {
 public:
  virtual void initialize(nlohmann::json& config) = 0;
  virtual void finalize(ICommunicator_ptr worldComm, long currentTime) = 0;
  virtual void newComm(long id, const T& obj, ICommunicator_ptr comm) = 0;
  virtual void write(long id, const T& obj, long jid) = 0;
  virtual json getRunInfo() = 0;
  virtual bool supportsFetch() { return false; }
  virtual bool fetch(long id, long jid, json& obj) { return false; }
  virtual std::string getFileStub() = 0; 

  StreamWriter(){};
  virtual ~StreamWriter(){

  };
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

  std::string getId() { return std::to_string(id++); }

  virtual void write(T& j) {
    // j[JSD::time] = VnV::RunTime::instance().currentTime();
    if (stream != nullptr) {
      stream->write(comm->uniqueId(), j, id++);
    }
  }

  virtual void syncId() { id = getNextId(comm, id); }

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
        // nJson[JSD::time] = VnV::RunTime::instance().currentTime();

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

  json getRunInfo() override { return stream->getRunInfo(); }

  bool Fetch(std::string message, const json& schema, long timeoutInSeconds, json& response) override {
    std::string line = response.dump();
    long size = -1;

    if (comm->Rank() == getRoot()) {
      response = json::object();
      bool worked = fetch(comm->uniqueId(), message, schema, timeoutInSeconds, response, id++);
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
      j[JSD::node] = JSN::shape;                                                       \
      if (!m.empty()) {                                                                \
        j[JSD::meta] = m;                                                              \
      }                                                                                \
      write(j);                                                                        \
    }                                                                                  \
  }
  LTypes
#undef X
#undef LTypes

      void
      Put(std::string variableName, IDataType_ptr data, const MetaData& m) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::dataTypeStarted;
      j[JSD::name] = variableName;
      if (!m.empty()) {
        j[JSD::meta] = m;
      }
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
                      std::vector<int> sizes, std::vector<int> offset, const MetaData& m) override {
    VnV::DataTypeCommunication d(comm);

    IDataType_ptr mainDT = DataTypeStore::instance().getDataType(dtype);

    // Gather all on the root processor
    IDataType_vec gather = d.GatherV(data, dtype, getRoot(), gsizes, sizes, offset, false);

    if (gather.size() > 0 && comm->Rank() == getRoot()) {
      IDataType_ptr mainDT = DataTypeStore::instance().getDataType(dtype);
      std::map<std::string, PutData> types = mainDT->getLocalPutData();
      if (types.size() == 0) {
        return;
      }

      if (types.size() != 1 || (*types.begin()).second.shape.size() > 1) {
        try {
          json j;
          j[JSD::name] = variableName;
          j[JSD::node] = JSN::shape;
          j[JSD::shape] = gsizes;
          j[JSD::dtype] = JST::GlobalArray;
          j[JSD::key] = dtype;
          if (!m.empty()) {
            j[JSD::meta] = m;
          }
          j[JSD::children] = WriteDataJson<json>(gather);
          write(j);
        } catch (VnV::VnVExceptionBase& e) {
          VnV_Error(VNVPACKAGENAME, "Could not Write Global Array: %s", e.what());
        }
      } else {
        // Just one value -- so lets just make this a param array -- saves space and all.  array

        std::string param = (*types.begin()).first;
        PutData& pdata = (*types.begin()).second;

        auto sdt = pdata.datatype;
        std::string typeF = "";
        T vals = T::array();

        if (sdt == SupportedDataType::DOUBLE) {
          typeF = JST::Double;
          for (auto it : gather) {
            if (it != nullptr) {
              double* aa = (double*)it->getPutData(param);
              vals.push_back(*aa);
            } else {
              vals.push_back(0.0);
            }
          }
        } else if (sdt == SupportedDataType::STRING) {
          typeF = JST::String;
          for (auto it : gather) {
            if (it != nullptr) {
              std::string* aa = (std::string*)it->getPutData(param);
              vals.push_back(*aa);
            } else {
              vals.push_back("");
            }
          }
        } else if (sdt == SupportedDataType::JSON) {
          typeF = JST::Json;
          for (auto it : gather) {
            if (it != nullptr) {
              json* aa = (json*)it->getPutData(param);
              vals.push_back(*aa);
            } else {
              vals.push_back(json::object());
            }
          }
        } else if (sdt == SupportedDataType::LONG) {
          typeF = JST::Long;
          for (auto it : gather) {
            if (it != nullptr) {
              long* aa = (long*)it->getPutData(param);
              vals.push_back(*aa);
            } else {
              vals.push_back(0);
            }
          }
        }

        T j = T::object();
        j[JSD::name] = variableName;
        j[JSD::dtype] = typeF;
        j[JSD::value] = vals;
        j[JSD::shape] = gsizes;
        j[JSD::node] = JSN::shape;
        if (!m.empty()) {
          j[JSD::meta] = m;
        }
        write(j);
      }
    }
  }

  void Log(ICommunicator_ptr logcomm, const char* package, int stage, std::string level, std::string message) override {
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
      log[JSD::name] = std::to_string(id++);
      log[JSD::comm] = comm->uniqueId();
      write(log);
    }

    // Set the old comm
    if (commsave != nullptr) {
      setComm(commsave, false);
    }
  }

  void finalize(ICommunicator_ptr worldComm, long currentTime) override {
    setComm(worldComm, false);
    if (comm->Rank() == getRoot()) {
      T nJson = T::object();
      nJson[JSD::node] = JSN::finalTime;
      nJson[JSD::time] = currentTime;
      write(nJson);
    }

    stream->finalize(worldComm, currentTime);
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

  void setFromJson(ICommunicator_ptr worldComm, json& config) override {
    stream->initialize(config);
    setComm(worldComm, false);
  }

  void sendInfoNode(ICommunicator_ptr worldComm, const json& fullJson, const json& prov, std::string workflowName, std::string workflowJob ) override {
    setComm(worldComm, false);

    T node_map = getNodeMap(worldComm);

    if (comm->Rank() == getRoot()) {
      // This is the first one so we send over the info. .
      T nJson = T::object();
      nJson[JSD::title] = "VnV Simulation Report";
      nJson[JSD::date] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
      nJson[JSD::name] = "MainInfo";
      nJson[JSD::node] = JSN::info;
      nJson[JSD::worldSize] = comm->Size();
      nJson[JSD::spec] = fullJson;
      nJson[JSD::nodeMap] = node_map;
      nJson[JSD::mpiversion] = comm->VersionLibrary();
      nJson[JSD::prov] = prov;
      nJson[JSD::workflow] = workflowName;
      nJson[JSD::jobName] = workflowJob;
      nJson[JSD::filestub] = DistUtils::getAbsolutePath(stream->getFileStub());
      write(nJson);
    }
  }


  void file(ICommunicator_ptr comm, std::string packageName, std::string name, bool inputFile, std::string reader,
            std::string infilename, std::string outfilename) override {
    
    setComm(comm, false);

    if (comm->Rank() == getRoot()) {
            
      T j = T::object();
      j[JSD::node] = JSN::file;
      j[JSD::input] = inputFile;

      VnV::ProvFile pf;
      pf.reader = reader;
      pf.comm = comm->uniqueId();
      pf.package = packageName;
      pf.name = name;
      pf.copy = outfilename.size() > 0;
      pf.crc = VnV::HashUtils::crc32(infilename);
      
      if (pf.copy) {
        auto fstub = stream->getFileStub();
        std::string fname = (inputFile ? "/inputs/" : "/outputs/") + outfilename;
        VnV::DistUtils::copy_file(infilename, fstub + fname);
        pf.info = VnV::DistUtils::getLibInfo(fstub + fname,0);
        pf.filename = fname;
        
      } else {
        pf.filename = infilename;
        pf.info = VnV::DistUtils::getLibInfo(infilename,0);
      }
      
      

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

  void write_stdout_info(ICommunicator_ptr comm, const std::string& out) override {
      setComm(comm, false);
      T j = T::object();
      j[JSD::comm] = comm->uniqueId();
      j[JSD::value] = out;
      j[JSD::node] = JSN::stdout;
      write(j);
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

  void workflowCallback(std::string stage, ICommunicator_ptr comm, std::string package, std::string name,
                        const json& info) {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = stage;
      j[JSD::name] = name;
      j[JSD::package] = package;
      j[JSD::info] = info;
      write(j);
    }
  }

  void workflowStartedCallback(ICommunicator_ptr comm, std::string package, std::string name,
                               const json& info) override {
    setComm(comm, true);
    workflowCallback(JSN::workflowStarted, comm, package, name, info);
  };
  void workflowEndedCallback(ICommunicator_ptr comm, std::string package, std::string name, const json& info) {
    workflowCallback(JSN::workflowFinished, comm, package, name, info);
  };

  void workflowUpdatedCallback(ICommunicator_ptr comm, std::string package, std::string name, const json& info) {
    workflowCallback(JSN::workflowUpdated, comm, package, name, info);
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

  void initializationStartedCallBack(ICommunicator_ptr comm, std::string packageName) override {
    setComm(comm, true);

    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::initializationStarted;
      j[JSD::package] = packageName;
      write(j);
    }
  }
  void initializationEndedCallBack(std::string packageName) override {
    if (comm->Rank() == getRoot()) {
      T j = T::object();
      j[JSD::node] = JSN::initializationEnded;
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

template <typename V> class FileStream : public StreamWriter<V> {
 protected:
  std::string filestub;
  virtual ~FileStream() {}

  std::string getFileName(long id) { return getFileName_(filestub, std::to_string(id) + extension); }

  json getRunInfo() override {
    json j = json::object();
    j["reader"] = "file";
    j["filename"] = DistUtils::getAbsolutePath(filestub);
    return j;
  }

  std::string getFileStub() override {
    return filestub;
  }

  std::string getFileName_(std::string root, std::string fname) {
    return VnV::DistUtils::join({root, fname}, 0777, true, true);
  }

 public:
  virtual void initialize(json& config) override {
    std::string s = config["filename"].template get<std::string>();
    filestub = s;

    if (std::filesystem::exists(filestub) ) {
       if (!std::filesystem::is_directory(filestub) || !std::filesystem::is_empty(filestub) ) {
          throw "Error: If filename exists, it must be a empty Directory.";
       }
    }
    try {
        std::string fname = getFileName_(filestub,"__start__");
        std::ofstream file(fname);
        file.close();
        std::filesystem::remove(fname);
    } catch(...) {
       throw "Error: Could not write file in that directory.";
    }
  }

  static std::string getSchema() {
    return R"(
      { "type" : "object" , 
        "properties" : {
           "filename" : {"type":"string"}
        },
        "required" : ["filename"] 
      }
    )";
  };

  virtual void finalize(ICommunicator_ptr wcomm, long currentTime) override = 0;

  virtual void newComm(long id, const V& obj, ICommunicator_ptr comm) override  = 0;

  virtual void write(long id, const V& obj, long jid) override = 0;
};


}  // namespace Nodes
}  // namespace VnV

#endif
