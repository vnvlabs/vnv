

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
        "outputFile": {
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


AdiosEngineManager::AdiosEngineManager() = default;

AdiosEngineManager::~AdiosEngineManager() = default;

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

void AdiosEngineManager::Put(std::string variableName, IDataType_ptr data, const MetaData& m) {
  curr->Put(variableName,data,m, this,0);
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
  long nextId = getNextId(comm);  // Could do nullptr here as logs  are root only at the moment.
  curr->Log(nextId, package, stage, level, message);
}

std::string AdiosEngineManager::getFileName(std::string root, std::vector<std::string> fname, bool mkdir) {
  fname.insert(fname.begin(), root);
  std::string filename = fname.back();
  fname.pop_back();
  std::string fullname = DistUtils::join(fname, 0777, mkdir);
  return fullname + filename;
}

std::string AdiosEngineManager::getFileName(const std::vector<std::string>& fname, bool mkdir) {
  return getFileName(outfile,fname, mkdir );
}

void AdiosEngineManager::finalize(ICommunicator_ptr worldComm) {
  setComm(worldComm);


  for (auto it : routes) {
    it.second->finalize();
  }

  std::vector<long> allComms = commMapper.listAllComms(worldComm);
  if (worldComm->Rank() == getRoot()) {
     json j = json::object();
     for (auto it : allComms) {
       j[getFileName(it, false)] = it;
     }
    std::ofstream out(getMetaDataFileName(outfile));
    out << j.dump();
    out.close();
  }
}

std::string AdiosEngineManager::getMetaDataFileName(std::string dir) {
    return getFileName(dir, {"metadata.json"},false);
}

void AdiosEngineManager::setFromJson(ICommunicator_ptr worldComm, nlohmann::json& config) {

  bool debug = false;
  std::string configFile = "";

  if (config.find("debug") != config.end()) debug = config["debug"].get<bool>();

  if (config.find("outputFile") != config.end())
    outfile = config["outputFile"].get<std::string>();

  if (config.find("configFile") != config.end())
    configFile = config["configFile"].get<std::string>();

  if (configFile.empty())
    adios = new adios2::ADIOS(MPI_COMM_WORLD, debug);
  else
    adios = new adios2::ADIOS(configFile, MPI_COMM_WORLD, debug);

  bpWriter = adios->DeclareIO("BPWriter");
  bpWriter.AddTransport("File",{{"Library", "POSIX"}, {"Name", outfile.c_str()}});
  AdiosEngineImpl::Define(bpWriter);

  // Set the communicator to be the main communication. 0
  setComm(worldComm);

  // Write the beginining inforation.
  curr->writeInfo();

}

std::string AdiosEngineManager::getFileName(long long commId, bool mkdir) {
  return getFileName({std::to_string(comm->uniqueId()) + ".bp3"},mkdir );
}

 void AdiosEngineManager::setComm(const ICommunicator_ptr& comm) {
   setCommunicator(comm);
   commMapper.logComm(comm);
  long uid = comm->uniqueId();
  auto it = routes.find(uid);
  if (it == routes.end()) {
    std::string datafname = getFileName(comm->uniqueId(),true);

    MPI_Comm mpicomm = VnV::Communication::MPI::castToMPIComm(comm);

    curr = std::make_shared<AdiosEngineImpl>(bpWriter, datafname,
                                             mpicomm, comm->uniqueId(),
                                             comm->Rank() == 0);
    routes[uid] = curr;

  } else {
    curr = it->second;
  }

    
}

long AdiosEngineManager::getNextId(const ICommunicator_ptr& comm) {
  if (comm != nullptr) {
    id = commMapper.getNextId(comm, id);
  }
  return id++;
}

void AdiosEngineManager::injectionPointEndedCallBack(std::string id_,
                                                     InjectionPointType type_,
                                                     std::string stageId) {
  long nextId = getNextId(comm);
  curr->injectionPointEndedCallBack(nextId, id_, type_, stageId);
}

void AdiosEngineManager::injectionPointStartedCallBack(ICommunicator_ptr comm,
                                                       std::string packageName,
                                                       std::string id_,

                                                       InjectionPointType type_,
                                                       std::string stageId) {
  long nextId = getNextId(comm);
  curr->injectionPointStartedCallBack(nextId, packageName, id_, type_, stageId);
}

void AdiosEngineManager::testStartedCallBack(std::string packageName,
                                             std::string testName,
                                             bool internal, long uid) {
  curr->testStartedCallBack(packageName, testName, internal, uid);
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

nlohmann::json AdiosEngineManager::getConfigurationSchema() {
  return __adios_input_schema__;
}

std::string AdiosEngineManager::print() { return "Adios Engine Manager"; }



}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
