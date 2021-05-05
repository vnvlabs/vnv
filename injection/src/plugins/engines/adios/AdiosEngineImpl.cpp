
/** @file AdiosOutputEngineImpl.cpp **/
#include "plugins/engines/adios/AdiosEngineImpl.h"

#include "base/Utilities.h"
#include "base/Runtime.h"
#include <chrono>

using nlohmann::json;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

namespace {

template <typename T>
adios2::Variable<T> define(adios2::IO& io, std::string variable) {
  adios2::Variable<T> v = io.InquireVariable<T>(variable);
  if (v) {
    throw VnVExceptionBase("Exists");
  } else {
    return io.DefineVariable<T>(variable);
  }
}
template <typename T>
adios2::Variable<T> define_local(adios2::IO& io, std::string variable) {
  adios2::Variable<T> v = io.InquireVariable<T>(variable);
  if (v) {
    throw VnVExceptionBase("Exists");
  } else {
    return io.DefineVariable<T>(variable, {adios2::LocalValueDim});
  }
}
template <typename T>
adios2::Variable<T> define_global(adios2::IO& io, std::string variable,
                                  const std::vector<std::size_t>& gsize,
                                  const std::vector<std::size_t>& offsets,
                                  const std::vector<std::size_t>& sizes) {
  adios2::Variable<T> v = io.InquireVariable<T>(variable);
  if (v) {
    throw VnVExceptionBase("Exists");
  } else {
    return io.DefineVariable<T>(variable, gsize, offsets, sizes);
  }
}

void defineAll(adios2::IO &io) {
  define<long>(io, "id");
  define<int>(io, "isJson");
  define<std::string>(io, "type");
  define<std::string>(io, "json");
  define<std::string>(io, "string");
  define<int>(io, "bool");
  define<long long>(io, "longlong");
  define<double>(io, "double");
  define_local<std::string>(io, "localjson");
  define_local<std::string>(io, "localstring");
  define_local<int>(io, "localbool");
  define_local<long long>(io, "locallonglong");
  define_local<double>(io, "localdouble");
  define<std::string>(io, "name");
  define<std::string>(io, "metaData");
  define<int>(io, "stage");
  define_local<int>(io, "localstage");
  define<long>(io, "dtype");

  define<int>(io, "output");
  define<long>(io, "identity");
  define<int>(io, "stageVal");
  define<std::string>(io, "levelVal");
  define<std::string>(io, "message");
  define<std::string>(io, "package");
  define<std::string>(io, "dim");
  define<std::string>(io, "shape");
  define<std::string>(io, "sizes");
  define<std::string>(io, "offsets");
  define<std::string>(io, "test");
  define<int>(io, "internal");
  define<int>(io, "result");
  define<std::size_t>(io, "size");

  define_global<double>(io, "doublevector", {1}, {1}, {1});
  define_global<double>(io, "longvector", {1}, {1}, {1});
  define_global<std::size_t>(io, "stringvector", {}, {}, {1});


}

}  // namespace

void AdiosEngineImpl::Define(adios2::IO& io) {
  defineAll(io);
}

long AdiosEngineImpl::ADIOS_ENGINE_IMPL_VERSION=1L;

AdiosEngineImpl::AdiosEngineImpl(adios2::IO& io_, const std::string& dataFname,
                                 const std::string& metaFname, MPI_Comm comm_,
                                 long uid, int rootRank)
    : io(io_), comm(comm_) {

  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  root = (rootRank == rank);

  std::string variablename = "worldrank-" + std::to_string(uid);
  define_global<int>(io, variablename, {static_cast<unsigned long>(size)},{static_cast<unsigned long>(rank)}, {1});
  engine = io.Open(dataFname, adios2::Mode::Write, comm);

  engine.BeginStep();
  engine.Put(variablename, rank);
  if (root) {
    engine.Put("id", uid);
    engine.Put<std::size_t>("size", size);
  }
  engine.EndStep();
}

AdiosEngineImpl::~AdiosEngineImpl() = default;

void AdiosEngineImpl::Put(const json& value, bool local) {
  if (root) {
    engine.Put("type", "json");
  }
  if (local) {
    engine.Put("localjson", value.dump());
  } else if (root) {
    engine.Put("json", value.dump());
  }
}
void AdiosEngineImpl::Put(const std::string& value, bool local) {
  if (root) {
    engine.Put("type", "string");
  }
  if (local) {
    engine.Put("localstring", value);
  } else if (root) {
    engine.Put("string", value);
  }
}
void AdiosEngineImpl::Put(const double& value, bool local) {
  if (root) {
    engine.Put("type", "double");
  }
  if (local) {
    engine.Put("localdouble", value);
  } else if (root) {
    engine.Put("double", value);
  }
}
void AdiosEngineImpl::Put(const bool& value, bool local) {
  if (root) {
    engine.Put("type", "bool");
  }
  // adios does not support bools.
  int vv = value ? 1:0;
  if (local) {
    engine.Put("localbool", vv);
  } else if (root) {
    engine.Put("bool", vv);
  }
}
void AdiosEngineImpl::Put(const long long& value, bool local) {
  if (root) {
    engine.Put("type", "longlong");
  }
  if (local) {
    engine.Put("locallonglong", value);
  } else if (root) {
    engine.Put("longlong", value);
  }
}

void AdiosEngineImpl::Put(const std::string& variableName, const IDataType_ptr& ptr,
                          const MetaData& m, IOutputEngine* oengine,
                          int writerank) {
  engine.BeginStep();
  if (root) {



    engine.Put("stage", engineStep++);
    engine.Put("type", "dts");
    engine.Put("name", variableName);
    engine.Put("metaData", VnV::StringUtils::metaDataToJsonString(m));
    engine.Put("stage", engineStep);
    engine.Put("output", writerank);
    engine.Put("dtype", ptr->getKey());
  }
  engine.EndStep();

  ptr->Put(oengine);

  engine.BeginStep();
  if (root) {
    engine.Put("type", "dte");
  }
  engine.EndStep();
}

void AdiosEngineImpl::Log(long id, const char* package, int stage,
                          const std::string& level, const std::string& message, int rank) {
  if (root) {
    std::string s = package;

    engine.BeginStep();
    engine.Put("identity", id);
    engine.Put("stageVal", stage);
    engine.Put("levelVal", level);
    engine.Put("message", message);
    engine.Put("package", s);
    engine.EndStep();
  }
}

namespace {
template <typename T>
void setVec(adios2::Engine& engine, adios2::IO& io, IDataType_vec data,
            const std::string& varname, adios2::Dims& gsizes, adios2::Dims& offsets,
            adios2::Dims& sizes, bool setShape) {
  int count = 0;
  std::vector<T> ddata(data.size());
  for (const auto& it : data) {
    T* dd = (T*)it->getPutData(count++);
    ddata.push_back(*dd);
  }

  auto v = io.InquireVariable<T>("varname");
  if (setShape) {
    v.SetShape(gsizes);
  }
  v.SetSelection({offsets, sizes});
  engine.Put<T>(v, ddata.data());
}

template <typename T>
void setStringVec(adios2::Engine& engine, adios2::IO& io, const IDataType_vec& data) {
  // For strings, everyone has there own size.
  int count = 0;
  json jarray = json::array();
  for (const auto& it : data) {
    T* dd = (T*)it->getPutData(count++);
    jarray.push_back(*dd);
  }
  std::string s = jarray.dump();
  // Write a global array showing the length of all the strings.
  auto strl = io.InquireVariable<char>("stringvector");
  strl.SetSelection({{}, {s.size()}});
  engine.Put<char>(strl, s.data());
}

}  // namespace

void AdiosEngineImpl::PutGlobalArray(long long dtype, const std::string& variableName,
                                     const IDataType_vec& data,
                                     std::vector<size_t> gsizes,
                                     std::vector<std::size_t> sizes,
                                     std::vector<std::size_t> offset,
                                     const MetaData& m) {
  // Define the variables to fit the shapes and dimenstion for each proc.

  std::size_t gsz = gsizes.size();
  std::string gszs = std::to_string(gsz);
  adios2::Variable<std::size_t> dim =
      io.InquireVariable<std::size_t>("size-" + gszs);
  if (!dim) {
    io.DefineVariable<std::size_t>("size-" + std::to_string(gsz), {gsz * size},
                                   {gsz * rank}, {gsz});
  }
  dim = io.InquireVariable<std::size_t>("offset-" + gszs);
  if (!dim) {
    io.DefineVariable<std::size_t>("offset-" + std::to_string(gsz),
                                   {gsz * size}, {gsz * rank}, {gsz});
  }
  dim = io.InquireVariable<std::size_t>("shape-" + gszs);
  if (!dim) {
    io.DefineVariable<std::size_t>("shape-" + gszs, {gsz}, {0},
                                   {gsz * (root ? 1 : 0)});
  }

  IDataType_ptr d = CommunicationStore::instance().getDataType(dtype);
  std::vector<PutData> types = d->getLocalPutData();

  engine.BeginStep();

  if (root) {
    // Write out some metadata for the global array.
    engine.Put("name", variableName);
    engine.Put("type", "vector");
    engine.Put("stage", engineStep);
    engine.Put("metadata", StringUtils::metaDataToJsonString(m));
    engine.Put("dim", gsz);
  }

  engine.Put<unsigned long>("shape-" + gszs, gsizes.data());
  engine.Put<unsigned long>("offsets-" + gszs, offset.data());
  engine.Put<unsigned long>("sizes-" + gszs, sizes.data());

  engine.EndStep();

  // Now we need to write the values.
  for (auto& it : types) {
    engine.BeginStep();

    if (root) {
      int vv = it.datatype == SupportedDataType::JSON ? 1:0;

      engine.Put("variableName", it.name);
      engine.Put("isJson", vv);
    }

    switch (it.datatype) {
    case SupportedDataType::DOUBLE: {
      setVec<double>(engine, io, data, "doublevector", gsizes, offset, sizes,
                     true);
      break;
    }
    case SupportedDataType::LONG: {
      setVec<long>(engine, io, data, "longvector", gsizes, offset, sizes, true);
      break;
    }
    case SupportedDataType::STRING: {
      setStringVec<std::string>(engine, io, data);
      break;
    }
    case SupportedDataType::JSON: {
      setStringVec<json>(engine, io, data);
      break;
    }
    }
    engine.EndStep();
  }

  engine.BeginStep();
  if (root) {
    // Write out some metadata for the global array.
    engine.Put("name", variableName);
    engine.Put("type", "vector_finished");
   }
   engine.EndStep();

}

void AdiosEngineImpl::injectionPointEndedCallBack(const std::string& id,
                                                  InjectionPointType type_,
                                                  const std::string& stageId) {
  // TODO Error Checking  -- We should be able to catch nesting errors at
  // runtime.
  engine.BeginStep();
  if (root) {
    engine.Put("type", "ipe");
  }
  engine.EndStep();

}

void AdiosEngineImpl::injectionPointStartedCallBack(long nextId,
                                                    const std::string& packageName,
                                                    const std::string& id,
                                                    InjectionPointType type_,
                                                    const std::string& stageId) {
  engine.BeginStep();
  if (root) {
    engine.Put("identity", nextId);

    if (type_ == InjectionPointType::Begin ||
        type_ == InjectionPointType::Single) {
      engine.Put("type", "ipbb");
      engine.Put("id", id);
      engine.Put("package", packageName);
    } else if (type_ == InjectionPointType::Iter) {
      engine.Put("type", "ipib");
      engine.Put("id", stageId);
    } else if (type_ == InjectionPointType::End) {
      engine.Put("type", "ipeb");
    }
    engine.EndStep();
  }
}

void AdiosEngineImpl::writeCommMap(const json& commMap) {
  engine.BeginStep();
  if (root) {
    engine.Put("type", "commMap");
    engine.Put("json", commMap.dump());
  }
  engine.EndStep();
}

void AdiosEngineImpl::writeInfo() {
  int worldSize, worldRank;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

  engine.BeginStep();
  if (worldRank == 0) {

    json info = json::object();
    info["title"] = "VnV Simulation Report";
    info["date"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
    info["spec"] = RunTime::instance().getFullJson();
    info["engine"] = "Adios";
    info["commsize"] = worldSize;
    info["version"] = ADIOS_ENGINE_IMPL_VERSION;
    engine.Put("json", info.dump());
  }
  engine.EndStep();
}


void AdiosEngineImpl::testStartedCallBack(const std::string& packageName,
                                          const std::string& testName, bool internal) {

  engine.BeginStep();
  if (root) {
    engine.Put("package", packageName);
    engine.Put("test", testName);
    engine.Put("type", "ts");

    int vv = internal ? 1:0;
    engine.Put("internal",vv);
  }
  engine.EndStep();

}

void AdiosEngineImpl::testFinishedCallBack(bool result_) {
  engine.BeginStep();
  if (root) {
    engine.Put("type", "te");

    int vv = result_ ? 1:0;
    engine.Put("result", vv);
  }
  engine.EndStep();

}

void AdiosEngineImpl::unitTestStartedCallBack(const std::string& packageName,
                                              const std::string& unitTestName) {
  engine.BeginStep();
  if (root) {
    engine.Put("package", packageName);
    engine.Put("test", unitTestName);
    engine.Put("type", "uts");
  }
  engine.EndStep();

}

void AdiosEngineImpl::unitTestFinishedCallBack(IUnitTest* tester) {
  engine.BeginStep();

  if (root) {
    engine.Put("type", "utr");

    VnV::UnitTestResults r = tester->getResults();
    for (auto it : r) {

      int res = std::get<2>(it) ? 1 : 0 ;
      engine.Put("name", std::get<0>(it));
      engine.Put("message", std::get<1>(it));
      engine.Put("result", res);
    }

  }
  engine.EndStep();


}


void AdiosEngineImpl::finalize() {
  if (engine) {
    engine.Close();
  }
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
