#include <fstream>

#include "adios2.h"
#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/stores/CommunicationStore.h"
#include "interfaces/ICommunicator.h"
#include "interfaces/IOutputEngine.h"
#include "plugins/comms/MPICommunicator.h"
#include "plugins/engines/adios_patch/ifstream.h"
#include "streaming/dispatch.h"

using namespace VnV::Nodes;
using nlohmann::json;

class IndexIter {
 public:
  std::size_t currInd = -1;
  std::vector<std::size_t> local;
  const std::vector<std::size_t>& shape;
  const std::vector<std::size_t>& offset;
  const std::vector<std::size_t>& size;
  IndexIter(const std::vector<std::size_t>& s, const std::vector<std::size_t>& off, const std::vector<std::size_t>& sze)
      : shape(s), offset(off), size(sze) {
    local.resize(off.size(), 0);
    set();
  }

  std::size_t get() const { return currInd; }

  std::size_t operator++(int x) {
    std::size_t ctemp = currInd;
    for (std::size_t i = 0; i < x; i++) {
      up();
    }
    return ctemp;
  }

  std::size_t operator++() {
    up();
    return currInd;
  }

  void up() {
    for (std::size_t i = size.size() - 1; i > -1; i--) {
      local[i]++;
      if (local[i] >= size.size()) {
        local[i] = 0;
      } else {
        set();
        return;
      }
    }
    currInd = -1;
  }

  void set() {
    currInd = 0;
    std::size_t currMult = 1;
    for (std::size_t i = shape.size() - 1; i > -1; i--) {
      currInd += currMult * (offset[i] + local[i]);
      currMult *= shape[i];
    }
  }

  int count() { return std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>()); }
};
#define STREAM_READER_NO_MORE_VALUES -19999

// A static file iterator.
class AdiosFileIterator : public Iterator<json> {
  adios2::ifstream fstream;
  adios2::ifstream step;

  long sId;
  std::ifstream ifs;
  std::string type;
  bool more;
  long currId;
  std::string currJson;

  json nextCurr;
  long nextValue = -1;

  void getLine_() {
    std::string currline;

    if (getstep(fstream, step, 0)) {  // blocking !!!!

      nextValue = step.read<long>("jid")[0];
      type = step.read<std::string>("type")[0];

      std::string s = step.read<std::string>("data")[0];
      nextCurr = json::parse(s);
      if (type.compare("a") == 0) {
        std::size_t dim = step.read<std::size_t>("dim")[0];
        std::vector<std::size_t> shape = step.read<std::size_t>("shape");
        std::vector<std::size_t> offsets = step.read<std::size_t>("offsets");
        std::vector<std::size_t> sizes = step.read<std::size_t>("sizes");

        int count = std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>());
        long long dtype = step.read<long long>("dtype")[0];

        nextCurr[JSD::children] = json::array();
        json& children = nextCurr[JSD::children];
        children.get_ptr<json::array_t*>()->reserve(count);

        std::vector<char> jvec = step.read<char>("globalVec");
        std::vector<int> jvecSize = step.read<int>("globalVecSize");
        std::size_t cummsum = 0;
        std::vector<char>::iterator iter = jvec.begin();
        std::vector<std::size_t>::iterator offiter = offsets.begin();
        std::vector<std::size_t>::iterator sizeiter = sizes.begin();

        for (std::size_t i = 0; i < jvecSize.size(); i++) {
          // Pull out the string for this "processor"
          std::vector<char>::iterator iterend = iter + jvecSize[i];
          std::string jstr(iter, iterend);
          iter = iterend;
          json jdata = json::parse(jstr);

          std::vector<std::size_t> loffset(offiter, offiter + dim);
          offiter += dim;

          std::vector<std::size_t> lsize(sizeiter, sizeiter + dim);
          sizeiter += dim;

          IndexIter indexiter(shape, loffset, lsize);
          for (auto& it : jdata.items()) {
            children.at(indexiter++) = it;
          }
        }
        step.end_step();
      }
    } else {
      nextValue = STREAM_READER_NO_MORE_VALUES;
      nextCurr = json::object();
      more = false;
    }
  }

  void getLine(json& current, long& currentValue) override {
    current = nextCurr;
    currentValue = nextValue;
    getLine_();
    return;
  }

 public:
  AdiosFileIterator(long streamId_, std::string filename_) : sId(streamId_), fstream(filename_) { getLine_(); }

  bool hasNext() override {
    if (nextValue == STREAM_READER_NO_MORE_VALUES) {
      getLine_();
    }
    return nextValue != STREAM_READER_NO_MORE_VALUES;
  }

  long streamId() const override { return sId; }

  ~AdiosFileIterator() { ifs.close(); }
};

class AdiosFileStream : public FileStream<AdiosFileIterator, json> {
 public:
  std::unique_ptr<adios2::ADIOS> adios;
  adios2::IO io;

  std::map<long, adios2::Engine> streams;
  std::map<long, int> commSize;
  std::map<long, int> commRank;

  std::string tstr = "";

  static std::string getSchema() {
    return R"({
      "type" : "object",
      "properties" : {
        "filename" : {"type" : "string" },
        "debug" : {"type" : "boolean" },
        "outputFile" : {"type" : "string" },
        "configFile" : {"type" : "string" }
      },
      "required" : ["filename"]
    })";
  }

  virtual void initialize(json& config) override {
    this->filestub = config["filename"].get<std::string>();
    bool debug = false;
    std::string configFile = "";

    if (config.find("debug") != config.end()) debug = config["debug"].get<bool>();

    if (config.find("outputFile") != config.end()) filestub = config["outputFile"].get<std::string>();

    if (config.find("configFile") != config.end()) configFile = config["configFile"].get<std::string>();

    if (configFile.empty())
      adios = std::make_unique<adios2::ADIOS>(MPI_COMM_WORLD, debug);
    else
      adios = std::make_unique<adios2::ADIOS>(configFile, MPI_COMM_WORLD, debug);

    io = adios->DeclareIO("BPWriter");

    io.DefineVariable<std::string>("data");
    io.DefineVariable<long>("jid");
    io.DefineVariable<std::string>("type");

    io.AddTransport("File", {{"Library", "POSIX"}, {"Name", filestub.c_str()}});
  }

  static std::string getResponseFileName(std::string stub, long id, long jid) {
    return VnV::DistUtils::join({stub, "__response__", std::to_string(jid) + "_" + std::to_string(id)}, 0777, true);
  }

  virtual bool supportsFetch() override { return true; }

  virtual bool fetch(long id, long jid, json& response) override {
    std::string s = getResponseFileName(filestub, id, jid);
    std::ifstream ifs(s + ".complete");
    if (ifs.good()) {
      response = json::parse(ifs);
      return true;
    }
    return false;
  }

  virtual void finalize(ICommunicator_ptr wcomm, long currentTime) override {
    // Close all the streams
    for (auto& it : streams) {
      json j = json::object();
      j[JSD::node] = JSN::done;
      j[JSD::time] = currentTime;
      write(it.first, j, -1);
      it.second.Close();
    }
  }

  virtual void newComm(long id, const json& obj, ICommunicator_ptr comm) override {
    if (streams.find(id) == streams.end()) {
      MPI_Comm cmm = VnV::Communication::MPI::castToMPIComm(comm);
      adios2::Engine e = io.Open(getFileName(id), adios2::Mode::Write, cmm);
      streams.insert(std::make_pair(id, std::move(e)));

      int size = 0;
      MPI_Comm_size(cmm, &size);
      commSize[id] = size;
      MPI_Comm_rank(cmm, &size);
      commRank[id] = size;

      write(id, obj, -1);
    }
  };

  virtual void write(long id, const json& obj, long jid) override {
    auto it = streams.find(id);
    if (it != streams.end()) {
      tstr = "j";
      it->second.BeginStep();
      it->second.Put("data", obj.dump());
      it->second.Put("jid", jid);
      it->second.Put("type", tstr);
      it->second.EndStep();
    }
  };

  virtual void writeGlobal(long commId, long jid, long long dtype, std::string variableName, VnV::IDataType_vec data,
                           std::vector<std::size_t> gsizes, std::vector<std::size_t> sizes,
                           std::vector<std::size_t> offset, const VnV::MetaData& m) {
    auto it = streams.find(commId);
    if (it != streams.end()) {
      writeGlobal(it->second, commId, jid, dtype, variableName, data, gsizes, sizes, offset, m);
    }
  }

  virtual void writeGlobal(adios2::Engine& engine, long commId, long jid, long long dtype, std::string variableName,
                           VnV::IDataType_vec data, std::vector<std::size_t> gsizes, std::vector<std::size_t> sizes,
                           std::vector<std::size_t> offset, const VnV::MetaData& m) {
    std::size_t size = commSize[commId];
    std::size_t rank = commRank[commId];
    bool root = (rank == 0);

    std::size_t gsz = gsizes.size();
    std::string gszs = std::to_string(gsz);

    adios2::Variable<std::size_t> dim = io.InquireVariable<std::size_t>("sizes");
    if (!dim) {
      io.DefineVariable<std::size_t>("sizes", {gsz * size}, {gsz * rank}, {gsz});
    } else {
      dim.SetShape({gsz * size});
      dim.SetSelection({{gsz * rank}, {gsz}});
    }
    dim = io.InquireVariable<std::size_t>("offset");
    if (!dim) {
      io.DefineVariable<std::size_t>("offset", {gsz * size}, {gsz * rank}, {gsz});
    } else {
      dim.SetShape({gsz * size});
      dim.SetSelection({{gsz * rank}, {gsz}});
    }

    dim = io.InquireVariable<std::size_t>("shape");
    if (!dim) {
      io.DefineVariable<std::size_t>("shape", {gsz}, {0}, {gsz * (root ? 1 : 0)});
    } else {
      dim.SetShape({gsz});
      dim.SetSelection({{0}, {gsz * (root ? 1 : 0)}});
    }

    engine.BeginStep();

    if (root) {
      tstr = "a";
      engine.Put("jid", jid);
      engine.Put("type", tstr);

      json j = json::object();
      j[JSD::node] = JSN::shape;
      j[JSD::name] = variableName;
      j[JSD::meta] = m;
      j[JSD::shape] = gsizes;
      j["dim"] = gsz;
      j["dtype"] = dtype;

      engine.Put("data", j.dump());
    }
    engine.Put<unsigned long>("shape", gsizes.data());
    engine.Put<unsigned long>("offset", offset.data());
    engine.Put<unsigned long>("sizes", sizes.data());

    std::string jstr;
    try {
      jstr = WriteDataJson<json>(data).dump();
    } catch (VnV::VnVExceptionBase& e) {
      VnV_Error(VNVPACKAGENAME, "Could not write global array: %s", e.what());
      return;
    }

    adios2::Variable<char> v = io.InquireVariable<char>("globalVec");
    if (!v) {
      v = io.DefineVariable<char>("globalVec", {adios2::JoinedDim}, {}, {jstr.size()});
    } else {
      v.SetSelection({{}, {jstr.size()}});
    }
    engine.Put(v, jstr.data());

    adios2::Variable<int> v1 = io.InquireVariable<int>("globalVecSize");
    if (!v1) {
      v1 = io.DefineVariable<int>("globalVecSize", {1}, {rank}, {1});
    } else {
      v1.SetSelection({{rank}, {1}});
    }
    engine.Put<int>(v1, jstr.size());

    engine.EndStep();
  }
};

class AdiosStreamManager : public StreamManager<json> {
  std::shared_ptr<AdiosFileStream> gs() { return std::dynamic_pointer_cast<AdiosFileStream>(stream); }

 public:
  AdiosStreamManager() : StreamManager<json>(std::make_shared<AdiosFileStream>()) {}

  virtual void PutGlobalArray(long long dtype, std::string variableName, VnV::IDataType_vec data,
                              std::vector<int> gsizes, std::vector<int> sizes, std::vector<int> offset,
                              const VnV::MetaData& m) override {
    std::vector<std::size_t> gsizes_(gsizes.begin(), gsizes.end());
    std::vector<std::size_t> sizes_(sizes.begin(), sizes.end());
    std::vector<std::size_t> offset_(offset.begin(), offset.end());

    gs()->writeGlobal(comm->uniqueId(), StreamManager<json>::id++, dtype, variableName, data, gsizes_, sizes_, offset_,
                      m);
  }
};

class MultiAdiosStreamIterator : public MultiStreamIterator<AdiosFileIterator, json> {
  std::set<std::string> loadedFiles;
  std::string filestub;
  std::string response_stub = "";

 public:
  MultiAdiosStreamIterator(std::string fstub) : MultiStreamIterator<AdiosFileIterator, json>(), filestub(fstub) {
    bool changed = false;

    while (VnV::DistUtils::fileExists(filestub)) {
      filestub = fstub + VnV::TimeUtils::timestamp();
      changed = true;
    }

    if (changed) {
      VnV_Warn(VNVPACKAGENAME, "Output will be written to %s because %s already exists", filestub.c_str(),
               fstub.c_str());
    }
  }

  virtual void respond(long id, long jid, const json& response) override {
    // Respond to a file request in a format that will be understood by rhe Json Stream that is
    // waiting for it.
    std::string s = AdiosFileStream::getResponseFileName(filestub, id, jid);
    std::ofstream ofs(s + ".responding");
    ofs << response.dump();
    ofs.close();
    VnV::DistUtils::mv(s + ".responding", s + ".complete");
  }

  void updateStreams() override {
    std::vector<std::string> files = VnV::DistUtils::listFilesInDirectory(filestub);
    for (auto& it : files) {
      if (it.size() > std::strlen(extension) && loadedFiles.find(it) == loadedFiles.end()) {
        loadedFiles.insert(it);
        std::size_t dot = it.find_last_of(".");
        try {
          if (dot != std::string::npos && it.substr(dot).compare(extension) == 0) {
            long id = std::atol(it.substr(0, dot).c_str());
            std::string fname = VnV::DistUtils::join({filestub, it}, 0777, false);
            add(std::make_shared<AdiosFileIterator>(id, fname));
          }
        } catch (...) {
        }
      }
    }
  }
};

INJECTION_ENGINE(VNVPACKAGENAME, adios_file, AdiosFileStream::getSchema()) { return new AdiosStreamManager(); }

INJECTION_ENGINE_READER(VNVPACKAGENAME, adios_file, VnV::Nodes::dispathSchema()) {
  auto stream = std::make_shared<MultiAdiosStreamIterator>(filename);
  return engineReaderDispatch<MultiAdiosStreamIterator, json>(async, config, stream, false);
}