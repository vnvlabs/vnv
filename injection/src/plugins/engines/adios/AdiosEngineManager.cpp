
/** @file AdiosOutputEngineImpl.cpp **/
#include "plugins/engines/adios/AdiosEngineManager.h"

#include <chrono>
#include <fstream>

#include "base/DistUtils.h"
#include "base/Runtime.h"
#include "plugins/comms/MPICommunicator.h"

using nlohmann::json;

static json __adios_input_schema__ = R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "$id": "http://rnet-tech.net/vv.schema.json",
    "title": "Adios Engine Input Schema",
    "description": "Schema for the adios Engine",
    "type": "object",
    "properties": {
        "debug": {
            "type": "boolean"
        },
        "configFile": {
            "type": "string"
        },
        "outFile": {
            "type": "string"
        }
    },
    "additionalProperties": false
})"_json;

INJECTION_ENGINE(VNVPACKAGENAME, adios) {
  return new VnV::VNVPACKAGENAME::Engines::AdiosEngineManager();
}

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

int AdiosEngineManager::ADIOS_ENGINE_MANAGER_VERSION = 1;

AdiosEngineManager::AdiosEngineManager() {}

AdiosEngineManager::~AdiosEngineManager() {}

void AdiosEngineManager::Put(std::string variableName, const double& value,
                             const MetaData& m) {
  curr->Put(variableName, value, m, 0);
}

void AdiosEngineManager::Put(std::string variableName, const long long& value,
                             const MetaData& m) {
  curr->Put(variableName, value, m, 0);
}

void AdiosEngineManager::Put(std::string variableName, const bool& value,
                             const MetaData& m) {
  curr->Put(variableName, value, m, 0);
}

void AdiosEngineManager::Put(std::string variableName,
                             const nlohmann::json& value, const MetaData& m) {
  curr->Put(variableName, value, m, 0);
}

void AdiosEngineManager::Put(std::string variableName, const std::string& value,
                             const MetaData& m) {
  curr->Put(variableName, value, m, 0);
}

void AdiosEngineManager::PutGlobalArray(
    long long dtype, std::string variableName, IDataType_vec data,
    std::vector<int> gsizes, std::vector<int> sizes, std::vector<int> offset,
    const MetaData& m) {
  std::vector<std::size_t> gsizes_(gsizes.begin(), gsizes.end());
  std::vector<std::size_t> sizes_(sizes.begin(), sizes.end());
  std::vector<std::size_t> offset_(offset.begin(), offset.end());

  curr->PutGlobalArray(dtype, variableName, data, gsizes_, sizes_, offset_, m);
}

void AdiosEngineManager::Log(ICommunicator_ptr comm, const char* package,
                             int stage, std::string level,
                             std::string message) {
  setComm(comm);
  long nextId = getNextId(
      comm);  // Could do nullptr here as logs  are root only at the moment.
  curr->Log(nextId, package, stage, level, message);
}

std::string AdiosEngineManager::getFileName(std::vector<std::string> fname) {
  fname.insert(fname.begin(), outfile);
  std::string filename = fname.back();
  fname.pop_back();
  std::string fullname = DistUtils::makeDirectories(fname, 0777);
  return fullname + filename;
}

void AdiosEngineManager::finalize(ICommunicator_ptr worldComm) {
  setComm(worldComm);

  // Finalize all the engines.
  for (auto it : routes) {
    it.second->finalize();
  }

  // Get the comm map information and write it to file.
  auto comms = commMapper.gatherCommInformation(worldComm);
  if (worldComm->Rank() == getRoot()) {
    if (comms.size() > 1) {
      throw VnVExceptionBase("To many root communicators");
    } else if (comms.size() == 1) {
      // Write the comm information to json/
      json jcomm = json::object();
      std::set<long> done1;
      for (auto it : comms) {
        it->toJson1(jcomm, done1);
      }

      json comMap = json::object();
      comMap["worldSize"] = worldComm->Size();
      comMap["map"] = jcomm;
      worldEngine.BeginStep();
      worldEngine.Put("commMap", comMap.dump());
      worldEngine.EndStep();
    }
    worldEngine.Close();
  }
}

void AdiosEngineManager::setFromJson(nlohmann::json& config) {
  bool debug = false;
  std::string configFile = "";

  if (config.find("debug") != config.end()) debug = config["debug"].get<bool>();

  if (config.find("outFile") != config.end())
    outfile = config["outFile"].get<std::string>();

  if (config.find("configFile") != config.end())
    configFile = config["configFile"].get<std::string>();

  if (configFile.empty())
    adios = new adios2::ADIOS(MPI_COMM_WORLD, debug);
  else
    adios = new adios2::ADIOS(configFile, MPI_COMM_WORLD, debug);

  bpWriter = adios->DeclareIO("BPWriter");
  outputFile = bpWriter.AddTransport("File", {{"Library", "POSIX"}, {"Name", outfile.c_str()}});

  bpWriter.DefineVariable<long>("identity");
  bpWriter.DefineVariable<double>("double");
  bpWriter.DefineVariable<long long>("longlong");
  bpWriter.DefineVariable<std::string>("string");
  bpWriter.DefineVariable<bool>("bool");
  bpWriter.DefineVariable<std::string>("json");
  bpWriter.DefineVariable<std::string>("metadata");
  bpWriter.DefineVariable<std::string>("name");
  bpWriter.DefineVariable<std::string>("package");
  bpWriter.DefineVariable<int>("stage");
  bpWriter.DefineVariable<std::string>("level");

  MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
  worldEngine = bpWriter.Open(getFileName({"info.json"}), adios2::Mode::Write,
                              MPI_COMM_WORLD);

  int worldSize;
  MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

  if (worldRank == 0) {
    json info = json::object();
    info["title"] = "VnV Simulation Report";
    info["date"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count();
    info["spec"] = RunTime::instance().getFullJson();
    info["engine"] = "Adios";
    info["commsize"] = worldSize;
    info["version"] = ADIOS_ENGINE_MANAGER_VERSION;

    worldEngine.BeginStep();
    worldEngine.Put("info", info.dump());
    worldEngine.EndStep();
  }
}

void AdiosEngineManager::setComm(ICommunicator_ptr comm) {
  commMapper.logComm(comm);
  long uid = comm->uniqueId();
  auto it = routes.find(uid);
  if (it == routes.end()) {
    std::string datafname =
        getFileName({"comms", std::to_string(comm->uniqueId()), "data.bp3"});
    std::string metafname =
        getFileName({"comms", std::to_string(comm->uniqueId()), "meta.bp3"});

    MPI_Comm mpicomm = VnV::Communication::MPI::castToMPIComm(comm);

    curr = std::make_shared<AdiosEngineImpl>(bpWriter, datafname, metafname,
                                             mpicomm, comm->uniqueId(),
                                             comm->Rank() == 0);
    routes[uid] = curr;

  } else {
    curr = it->second;
  }
}

long AdiosEngineManager::getNextId(ICommunicator_ptr comm) {
  if (comm != nullptr) {
    id = commMapper.getNextId(comm, id);
  }
  return id++;
}

void AdiosEngineManager::injectionPointEndedCallBack(std::string id,
                                                     InjectionPointType type_,
                                                     std::string stageId) {
  setComm(comm);
  curr->injectionPointEndedCallBack(id, type_, stageId);
}

void AdiosEngineManager::injectionPointStartedCallBack(ICommunicator_ptr comm,
                                                       std::string packageName,
                                                       std::string id,

                                                       InjectionPointType type_,
                                                       std::string stageId) {
  setComm(comm);
  long nextId = getNextId(comm);
  curr->injectionPointStartedCallBack(nextId, packageName, id, type_, stageId);
}

void AdiosEngineManager::testStartedCallBack(std::string packageName,
                                             std::string testName,
                                             bool internal) {
  curr->testStartedCallBack(packageName, testName, internal);
}

void AdiosEngineManager::testFinishedCallBack(bool result_) {
  curr->testFinishedCallBack(result_);
}

void AdiosEngineManager::unitTestStartedCallBack(ICommunicator_ptr comm,
                                                 std::string packageName,
                                                 std::string unitTestName) {
  setComm(comm);
  curr->unitTestStartedCallBack(packageName, unitTestName);
}

void AdiosEngineManager::unitTestFinishedCallBack(IUnitTest* tester) {
  setComm(comm);
  curr->unitTestFinishedCallBack(tester);
}
void AdiosEngineManager::dataTypeStartedCallBack(std::string variableName,
                                                 long long dtype,
                                                 const MetaData& m) {
  curr->dataTypeStartedCallBack(variableName, dtype, m);
}

void AdiosEngineManager::dataTypeEndedCallBack(std::string variableName) {
  curr->dataTypeEndedCallBack(variableName);
}
nlohmann::json AdiosEngineManager::getConfigurationSchema() {
  return __adios_input_schema__;
}

std::string AdiosEngineManager::print() { return "Adios Engine Manager"; }

Nodes::IRootNode* AdiosEngineManager::readFromFile(std::string, long&) {
  throw VnVExceptionBase("Reader Not Implemented for Adios Engine");
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
