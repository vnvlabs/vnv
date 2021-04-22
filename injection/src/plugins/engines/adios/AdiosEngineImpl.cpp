
/** @file AdiosOutputEngineImpl.cpp **/
#include "plugins/engines/adios/AdiosEngineImpl.h"

#include "base/Utilities.h"
using nlohmann::json;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

void AdiosEngineImpl::Declare(adios2::IO& io, std::set<std::string> exists) {



}



AdiosEngineImpl::AdiosEngineImpl(adios2::IO& io_, std::string dataFname, std::string metaFname,
                                 MPI_Comm comm_, long uid, int rootRank)
    : io(io_), comm(comm_) {
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  root = (rootRank == rank);
  std::string variablename = "worldrank-" + std::to_string(uid);

  io.DefineVariable<int>(variablename ,
                         {static_cast<unsigned long>(size)},
                         {static_cast<unsigned long>(rank)},
                         {1}
                         );

  engine = io.Open(dataFname, adios2::Mode::Write, comm);
  engine.BeginStep();

  engine.Put(variablename, &rank);
  engine.EndStep();

  // RootEngine is in charge of writing the heirarchy.
  // Basically -- the root engine writes all the metadata to the heirarchy file.
  // Any time there is a parallel write, we write to the "engine". We know how
  // many times we write to the engine file, so we know which "step" goes with
  // which data.

  if (root) {
    rootEngine = io.Open(metaFname, adios2::Mode::Write, MPI_COMM_SELF);
    rootEngine.BeginStep();
    rootEngine.Put("id", uid);
    rootEngine.Put("size", size);
    rootEngine.EndStep();
  }

}

AdiosEngineImpl::~AdiosEngineImpl() {}

void AdiosEngineImpl::Put(const json& value, bool local) {
  if (root) {
    rootEngine.Put("type", "json");
  }
  if (local) {
    engine.Put("localjson", value.dump());
  } else {
    engine.Put("json", value.dump());
  }
}
void AdiosEngineImpl::Put(const std::string& value, bool local) {
  if (root) {
    rootEngine.Put("type", "string");
  }
  if (local) {
    engine.Put("localstring", value);
  } else {
    engine.Put("string", value);
  }
}
void AdiosEngineImpl::Put(const double& value, bool local) {
  if (root) {
    rootEngine.Put("type", "double");
  }
  if (local) {
    engine.Put("localdouble", value);
  } else {
    engine.Put("double", value);
  }
}
void AdiosEngineImpl::Put(const bool& value, bool local) {
  if (root) {
    rootEngine.Put("type", "bool");
  }
  if (local) {
    engine.Put("localbool", value);
  } else {
    engine.Put("bool", value);
  }
}
void AdiosEngineImpl::Put(const long long& value, bool local) {
  if (root) {
    rootEngine.Put("type", "longlong");
  }
  if (local) {
    engine.Put("locallonglong", value);
  } else {
    engine.Put("longlong", value);
  }
}

void AdiosEngineImpl::Put(std::string variableName, IDataType_ptr ptr,
                          const MetaData& m, IOutputEngine* oengine,
                          int writerank) {
  if (root) {
    rootEngine.BeginStep();
    rootEngine.Put("name", variableName);
    rootEngine.Put("metaData", StringUtils::metaDataToJsonString(m));
    rootEngine.Put("stage", engineStep);
    rootEngine.Put("output", writerank);
    rootEngine.Put("type", "dt");
    rootEngine.EndStep();
  }

  bool write = (rank == writerank || writerank < 0);

  if (write) {
    engine.BeginStep();
    engine.Put("stage", engineStep++);
    engine.Put("type", "dts");
    engine.Put("dtype", ptr->getKey());
    engine.EndStep();

    // Data Type Put Methods are local only !!!!!!!
    // TODO How to enforce this?
    ptr->Put(oengine);  // ptr->Put(oengine, writeRank == raml  )

    engine.BeginStep();
    engine.Put("type", "dte");
    engine.EndStep();
  }
}

void AdiosEngineImpl::Log(long id, const char* package, int stage,
                          std::string level, std::string message, int rank) {
  if (root) {
    std::string s = package;
    engine.BeginStep();
    rootEngine.Put("identity", id);
    rootEngine.Put("stageVal", stage);
    rootEngine.Put("levelVal", level);
    rootEngine.Put("stringVal", message);
    rootEngine.Put("packageVal", s);
    rootEngine.EndStep();
  }
}

void AdiosEngineImpl::PutGlobalArray(std::string variableName,
                                     IDataType_vec local_data,
                                     std::vector<std::size_t> gsizes,
                                     std::vector<std::size_t> sizes,
                                     std::vector<std::size_t> offset,
                                     const MetaData& metadata) {
  if (root) {
    std::string s = "vector";
    rootEngine.BeginStep();
    rootEngine.Put("name", variableName);
    rootEngine.Put("type", "vector");
    rootEngine.Put("dtype", local_data[0]->getKey());
    rootEngine.Put("stage", engineStep);
    rootEngine.Put("metadata", StringUtils::metaDataToJsonString(metadata));
    rootEngine.Put("dim", gsizes.size());
    rootEngine.Put("shape", StringUtils::toString(gsizes));
    rootEngine.EndStep();
  }

  engine.BeginStep();
  engine.Put("type", "ba");
  engine.Put("size", local_data.size());
  engine.Put("sizes", StringUtils::toString(sizes));
  engine.Put("offsets", StringUtils::toString(offset));
  engine.Put("stage", engineStep++);
  engine.EndStep();

  for (auto it : local_data) {
    engine.BeginStep();
    engine.Put("type", "dts");
    engine.Put("dtype", it->getKey());
    engine.EndStep();

    it->Put(oengine);

    engine.BeginStep();
    engine.Put("type", "dte");
    engine.EndStep();
  }

  engine.BeginStep();
  engine.Put("type", "ea");
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
