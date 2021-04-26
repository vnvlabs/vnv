
/** @file AdiosOutputEngineImpl.cpp **/
#include "plugins/engines/adios/AdiosEngineImpl.h"

#include "base/Utilities.h"
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

}

AdiosEngineImpl::AdiosEngineImpl(adios2::IO& io_, std::string dataFname, std::string metaFname,
                                 MPI_Comm comm_, long uid, int rootRank)
    : io(io_), comm(comm_) {
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  root = (rootRank == rank);

  define<long>(io,"id");
  define<int>(io,"size");
  define<std::string>(io,"type");
  define<std::string>(io, "json");
  define<std::string>(io,"string");
  define<bool>(io,"bool");
  define<long long>(io,"longlong");
  define<double>(io,"double");
  define_local<std::string>(io, "localjson");
  define_local<std::string>(io,"localstring");
  define_local<bool>(io,"localbool");
  define_local<long long>(io,"locallonglong");
  define_local<double>(io,"localdouble");
  define<std::string>(io,"name");
  define<std::string>(io,"metaData");
  define<int>(io,"stage");
  define_local<int>(io,"localstage");
  define<long>(io,"dtype");

  define<int>(io,"output");
  define<long>(io,"identity");
  define<int>(io,"stageVal");
  define<std::string>(io,"levelVal");
  define<std::string>(io,"message");
  define<std::string>(io,"package");
  define<std::string>(io,"dim");
  define<std::string>(io,"shape");
  define<std::string>(io,"sizes");
  define<std::string>(io,"offsets");
  define<std::string>(io,"test");
  define<bool>(io,"internal");
  define<bool>(io,"result");
  define<std::size_t>(io,"size");

  std::string variablename = "worldrank-" + std::to_string(uid);
  define_global<int>(io, variablename, {static_cast<unsigned long>(size)},{static_cast<unsigned long>(rank)}, {1});

  engine = io.Open(dataFname, adios2::Mode::Write, comm);
  engine.BeginStep();

  engine.Put(variablename, &rank);

  engine.EndStep();

  engine.BeginStep();
  if (root) {
    engine.Put("id", uid);
    engine.Put("size", size);
  }
  engine.EndStep();

}

AdiosEngineImpl::~AdiosEngineImpl() {}

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
  } else if (root){
    engine.Put("double", value);
  }
}
void AdiosEngineImpl::Put(const bool& value, bool local) {
  if (root) {
    engine.Put("type", "bool");
  }
  if (local) {
    engine.Put("localbool", value);
  } else if (root){
    engine.Put("bool", value);
  }
}
void AdiosEngineImpl::Put(const long long& value, bool local) {
  if (root) {
    engine.Put("type", "longlong");
  }
  if (local) {
    engine.Put("locallonglong", value);
  } else if (root){
    engine.Put("longlong", value);
  }
}

void AdiosEngineImpl::Put(std::string variableName, IDataType_ptr ptr,
                          const MetaData& m, IOutputEngine* oengine,
                          int writerank) {


  engine.BeginStep();
  if (root) {
      engine.Put("stage", engineStep++);
      engine.Put("type", "dts");
      engine.Put("name", variableName);
      engine.Put("metaData", StringUtils::metaDataToJsonString(m));
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
                          std::string level, std::string message, int rank) {
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

void AdiosEngineImpl::PutGlobalArray(long long dtype, std::string variableName,
                    IDataType_vec data,std::vector<size_t> gsizes,
                    std::vector<std::size_t> sizes, std::vector<std::size_t> offset,
                    const MetaData& m)  {

  engine.BeginStep();
  if (root) {
    engine.Put("name", variableName);
    engine.Put("type", "vector");
    engine.Put("stage", engineStep);
    engine.Put("metadata", StringUtils::metaDataToJsonString(m));
    engine.Put("dim", gsizes.size());
    engine.Put("shape", StringUtils::toString(gsizes));
  }

  for (int i = 0; i < gsizes.size(); i++) {

    adios2::Variable<std::size_t> offsetv;
    adios2::Variable<std::size_t> dim =
        io.InquireVariable<std::size_t>("size-" + std::to_string(i));
    if (!dim) {
      dim = io.DefineVariable<std::size_t>("size-" + std::to_string(i),{adios2::LocalValueDim});
      offsetv = io.DefineVariable<std::size_t>("offset-" + std::to_string(i),{adios2::LocalValueDim});
    }
    engine.Put(dim, sizes[i]);
    engine.Put(offsetv, offset[i]);
  }

  IDataType_ptr d = CommunicationStore::instance().getDataType(dtype);
  std::vector<SupportedDataType> types = d->getLocalPutVariableCount();

  for (int i = 0; i < types.size(); i++ ) {

     int mycount = 0;
     if (data.size() == 0 ) {
       switch(types[i]) {
       case SupportedDataType::DOUBLE:
         io.DefineVariable<double>("todo", {},{},{0});
         break;
       case SupportedDataType::LONG:
         io.DefineVariable<long>("todo", {},{},{0});
         break;
       case SupportedDataType::JSON:
       case SupportedDataType::STRING:
         io.DefineVariable<char>("todo", {},{},{0});
         break;
       }
     }

     for (int j = 0; j < data.size(); j++ ) {

     }
     io.DefineVariable("todo", {}, {} , )
  }
  engine.EndStep();
  // Now we need to write the values.
  SupportedDataType t = SupportedDataType::STRING;

  auto types = CommunicationStore::instance().getLocalPutData(dtype);
  void* outdata;
  int count = 0;
  for (auto &it : types) {
    engine.BeginStep();
    if (root) {
      engine.Put("variableName", it.name);
    }

    switch (it.datatype) {

    case SupportedDataType::DOUBLE: {

      io.DefineVariable<double>("doublevalue", gsizes, offset, sizes);
      std::vector<double> ddata(data.size());
      for (auto it : data) {

        double* dd = (double*) outdata;
        ddata.push_back(*dd);
      }
      engine.Put("value", ddata.data());
      break;

    }

    case SupportedDataType::LONG: {
      io.DefineVariable<long>("longvalue", gsizes, offset, sizes);
      std::vector<long> ddata(data.size());
      for (auto it : data) {
        long* dd = (long*) it->LocalPut(count);
        ddata.push_back(*dd);
      }
      engine.Put("value", ddata.data());
      break;
    }

    case SupportedDataType::STRING:
      break;
    case SupportedDataType::JSON:
      break;
    }
    engine.EndStep();
    count++;
  }


  engine.EndStep();
}

void AdiosEngineImpl::injectionPointEndedCallBack(std::string id,
                                                  InjectionPointType type_,
                                                  std::string stageId) {
  // TODO Error Checking  -- We should be able to catch nesting errors at
  // runtime.
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("type", "ipe");
    rootEngine.EndStep();
  }
}

void AdiosEngineImpl::injectionPointStartedCallBack(long nextId,
                                                    std::string packageName,
                                                    std::string id,
                                                    InjectionPointType type_,
                                                    std::string stageId) {
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("identity", nextId);

    if (type_ == InjectionPointType::Begin ||
        type_ == InjectionPointType::Single) {
      rootEngine.Put("type", "ipbb");
      rootEngine.Put("id", id);
      rootEngine.Put("package", packageName);
    } else if (type_ == InjectionPointType::Iter) {
      rootEngine.Put("type", "ipib");
      rootEngine.Put("id", stageId);
    } else if (type_ == InjectionPointType::End) {
      rootEngine.Put("type", "ipeb");
    }
    rootEngine.EndStep();
  }
}

void AdiosEngineImpl::testStartedCallBack(std::string packageName,
                                          std::string testName, bool internal) {
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("package", packageName);
    rootEngine.Put("test", testName);
    rootEngine.Put("type", "ts");
    rootEngine.Put("internal", internal);
    rootEngine.EndStep();
  }
}

void AdiosEngineImpl::testFinishedCallBack(bool result_) {
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("type", "te");
    rootEngine.Put("result", result_);
    rootEngine.EndStep();
  }
}

void AdiosEngineImpl::unitTestStartedCallBack(std::string packageName,
                                              std::string unitTestName) {
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("package", packageName);
    rootEngine.Put("test", unitTestName);
    rootEngine.Put("type", "uts");
    rootEngine.EndStep();
  }
}

void AdiosEngineImpl::unitTestFinishedCallBack(IUnitTest* tester) {
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("type", "utrs");
    rootEngine.EndStep();

    VnV::UnitTestResults r = tester->getResults();
    for (auto it : r) {
      rootEngine.BeginStep();
      rootEngine.Put("name", std::get<0>(it));
      rootEngine.Put("message", std::get<1>(it));
      rootEngine.Put("result", std::get<2>(it));
      rootEngine.EndStep();
    }

    rootEngine.BeginStep();
    rootEngine.Put("type", "utre");
    rootEngine.EndStep();
  }
}
void AdiosEngineImpl::dataTypeStartedCallBack(std::string variableName,
                                              long long dtype,
                                              const MetaData& m) {}

void AdiosEngineImpl::dataTypeEndedCallBack(std::string variableName) {}

void AdiosEngineImpl::finalize() {
  if (engine) {
    engine.Close();
  }
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
