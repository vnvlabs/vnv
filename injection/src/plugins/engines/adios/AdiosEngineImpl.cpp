
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
  define<int>(io, "type");
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

  define_global<char>(io, "globalVec", {}, {}, {1});
  define_global<int>(io, "worldRanks", {1},{1}, {1});

}

}  // namespace

void AdiosEngineImpl::Define(adios2::IO& io) {
  defineAll(io);
}

long AdiosEngineImpl::ADIOS_ENGINE_IMPL_VERSION=1L;

AdiosEngineImpl::AdiosEngineImpl(adios2::IO& io_, const std::string& dataFname, MPI_Comm comm_,
                                 long uid, int rootRank)
    : io(io_), comm(comm_) {

  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  root = (rootRank == rank);

  int worldSize, worldRank;
  MPI_Comm_size(MPI_COMM_WORLD,&worldSize);
  MPI_Comm_rank(MPI_COMM_WORLD,&worldRank);

  engine = io.Open(dataFname, adios2::Mode::Write, comm);

  int w = (worldSize == size) ? 1 : 0;

  engine.BeginStep();
  if (root) {
    engine.Put("id", uid);
    engine.Put<std::size_t>("size", size);
    engine.Put<int>("bool", w);
  }

  if (!w) {
    auto t = io.InquireVariable<int>("worldRank");
    t.SetShape({static_cast<unsigned long>(size)});
    t.SetSelection({{static_cast<unsigned long>(rank)},{1}});
    engine.Put<int>(t, &worldRank);
  }

  engine.EndStep();
}

AdiosEngineImpl::~AdiosEngineImpl() = default;

void AdiosEngineImpl::type(AdiosDataType type) {
   engine.Put("type", static_cast<int>(type));
}

void AdiosEngineImpl::Put(const json& value, bool local) {
  if (root) {
    type(AdiosDataType::JSON);
  }
  if (local) {
    engine.Put("localjson", value.dump());
  } else if (root) {
    engine.Put("json", value.dump());
  }
}


void AdiosEngineImpl::Put(const std::string& value, bool local) {
  if (root) {
    type(AdiosDataType::STRING);
  }
  if (local) {
    engine.Put("localstring", value);
  } else if (root) {
    engine.Put("string", value);
  }
}
void AdiosEngineImpl::Put(const double& value, bool local) {
  if (root) {
    type(AdiosDataType::DOUBLE);
  }
  if (local) {
    engine.Put("localdouble", value);
  } else if (root) {
    engine.Put("double", value);
  }
}

void AdiosEngineImpl::Put(const bool& value, bool local) {
  if (root) {
    type(AdiosDataType::BOOL);
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
    type(AdiosDataType::LONGLONG);
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
    type(AdiosDataType::DATA_TYPE_START);
    engine.Put("name", variableName);
    engine.Put("metaData", VnV::StringUtils::metaDataToJsonString(m));
    engine.Put("output", writerank);
    engine.Put("dtype", ptr->getKey());
  }
  engine.EndStep();

  ptr->Put(oengine);

  engine.BeginStep();
  if (root) {
    type(AdiosDataType::DATA_TYPE_END);
  }
  engine.EndStep();
}

void AdiosEngineImpl::Log(long id, const char* package, int stage,
                          const std::string& level, const std::string& message, int rank) {
  if (root) {
    std::string s = package;

    engine.BeginStep();
    type(AdiosDataType::LOG);
    engine.Put("identity", id);
    engine.Put("stageVal", stage);
    engine.Put("levelVal", level);
    engine.Put("message", message);
    engine.Put("package", s);
    engine.EndStep();
  }
}

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
      io.InquireVariable<std::size_t>("size" );
  if (!dim) {
    io.DefineVariable<std::size_t>("size", {gsz * size},
                                   {gsz * rank}, {gsz});
  } else {
     dim.SetShape({ gsz * size });
     dim.SetSelection( {{gsz*rank},{gsz}});
  }
  dim = io.InquireVariable<std::size_t>("offset");
  if (!dim) {
    io.DefineVariable<std::size_t>("offset",
                                   {gsz * size}, {gsz * rank}, {gsz});
  } else {
    dim.SetShape({ gsz * size });
    dim.SetSelection( {{gsz*rank},{gsz}});
  }

  dim = io.InquireVariable<std::size_t>("shape");
  if (!dim) {
    io.DefineVariable<std::size_t>("shape", {gsz}, {0},
                                   {gsz * (root ? 1 : 0)});
  } else {
    dim.SetShape({gsz});
    dim.SetSelection({{0},{gsz*(root?1:0)}});
  }


  engine.BeginStep();

  if (root) {
    // Write out some metadata for the global array.
    engine.Put("name", variableName);
    type(AdiosDataType::VECTOR_START);
    engine.Put("stage", engineStep);
    engine.Put("metadata", StringUtils::metaDataToJsonString(m));
    engine.Put("dim", gsz);
    engine.Put("dtype", dtype);

  }

  engine.Put<unsigned long>("shape", gsizes.data());
  engine.Put<unsigned long>("offsets", offset.data());
  engine.Put<unsigned long>("sizes", sizes.data());

  // Now we need to write the values.
  IDataType_ptr d = CommunicationStore::instance().getDataType(dtype);
  std::map<std::string, PutData> types = d->getLocalPutData();

  json j = json::array();
  for (auto it : data ) {

    // Build a json object describing this data element.
    json cj;
    std::map<std::string, PutData> types = it->getLocalPutData();
    for (auto& it : types) {
      json childJson;
      PutData& p = it.second;
      void* outdata = d->getPutData(it.first);
      childJson["name"] = p.name;
      childJson["shape"] = p.shape;
      childJson["type"] = p.datatype;

      int count = std::accumulate(p.shape.begin(), p.shape.end(), 1,
                                  std::multiplies<>());

      switch (p.datatype) {
      case SupportedDataType::DOUBLE: {
        double* dd = (double*)outdata;
        std::vector<double> da(dd, dd + count);
        childJson["value"] = da;
        break;
      }

      case SupportedDataType::LONG: {
        long* dd = (long*)outdata;
        std::vector<long> da(dd, dd + count);
        childJson["value"] = da;
        break;
      }

      case SupportedDataType::STRING: {
        std::string* dd = (std::string*)outdata;
        std::vector<std::string> da(dd, dd + count);
        childJson["value"] = da;
        break;
      }

      case SupportedDataType::JSON: {
        json* dd = (json*)outdata;
        std::vector<json> da(dd, dd + count);
        json dad = json::array();
        for (int i = 0; i < count; i++,dd++ ) {
          dad.push_back(dd->dump());
        }
        childJson["value"] = dad;
        break;
      }

      }
      cj[p.name] = childJson;
    }
    j.push_back(cj);
  }
  std::string jstr = j.dump();
  adios2::Variable<char> v = io.InquireVariable<char>("globalVec");
  v.SetSelection({{},{jstr.size()}});
  engine.Put(v,jstr.data());

  engine.EndStep();

}

void AdiosEngineImpl::injectionPointEndedCallBack(long long nextId,
                                                  const std::string& id,
                                                  InjectionPointType type_,
                                                  const std::string& stageId) {
  // TODO Error Checking  -- We should be able to catch nesting errors at
  // runtime.
  engine.BeginStep();
  if (root) {
    engine.Put("identity", nextId);
    if (type_ == InjectionPointType::Begin) {
      type(AdiosDataType::INJECTION_POINT_BEGIN_END);
    } else if ( type_ == InjectionPointType::Single) {
      type(AdiosDataType::INJECTION_POINT_SINGLE_END);
    } else if (type_ == InjectionPointType::Iter) {
      type(AdiosDataType::INJECTION_POINT_ITER_END);
    } else if (type_ == InjectionPointType::End) {
      type(AdiosDataType::INJECTION_POINT_END_END);
    }
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

    if (type_ == InjectionPointType::Begin) {
      type(AdiosDataType::INJECTION_POINT_BEGIN_BEGIN);
      engine.Put("name", id);
      engine.Put("package", packageName);
    } else if (type_ == InjectionPointType::Single) {
      type(AdiosDataType::INJECTION_POINT_SINGLE_BEGIN);
      engine.Put("name", id);
      engine.Put("package", packageName);
    } else if (type_ == InjectionPointType::Iter) {
      type(AdiosDataType::INJECTION_POINT_ITER_BEGIN);
      engine.Put("id", stageId);
    } else if (type_ == InjectionPointType::End) {
      type(AdiosDataType::INJECTION_POINT_END_BEGIN);
    }
    engine.EndStep();
  }
}

void AdiosEngineImpl::writeInfo() {
  int worldSize, worldRank;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

  engine.BeginStep();
  if (worldRank == 0) {
    type(AdiosDataType::INFO);
    json info = json::object();

    info["title"] = "VnV Simulation Report";

    info["date"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
    info["spec"] = RunTime::instance().getFullJson();
    info["engine"] = "Adios";
    info["worldsize"] = worldSize;
    info["version"] = ADIOS_ENGINE_IMPL_VERSION;
    engine.Put("json", info.dump());
  }
  engine.EndStep();
}


void AdiosEngineImpl::testStartedCallBack(const std::string& packageName,
                                          const std::string& testName, bool internal, long uid) {

  engine.BeginStep();
  if (root) {
    engine.Put("package", packageName);
    engine.Put("test", testName);
    type(AdiosDataType::TEST_START);

    int vv = internal ? 1:0;
    engine.Put("internal",vv);
  }
  engine.EndStep();

}

void AdiosEngineImpl::testFinishedCallBack(bool result_) {
  engine.BeginStep();
  if (root) {
    type(AdiosDataType::TEST_END);

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
    type(AdiosDataType::UNIT_TEST_START);
  }
  engine.EndStep();

}

void AdiosEngineImpl::unitTestFinishedCallBack(IUnitTest* tester) {
  engine.BeginStep();

  if (root) {

    VnV::UnitTestResults r = tester->getResults();
    for (auto it : r) {

      int res = std::get<2>(it) ? 1 : 0 ;
      engine.Put("name", std::get<0>(it));
      engine.Put("message", std::get<1>(it));
      engine.Put("result", res);
    }
    type(AdiosDataType::UNIT_TEST_END);

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
