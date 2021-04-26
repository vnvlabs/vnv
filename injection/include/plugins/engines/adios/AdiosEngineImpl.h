#ifndef AdiosEngineImpl_H
#define AdiosEngineImpl_H

#include <map>
#include <sstream>
#include <string>

#include "adios2.h"
#include "base/CommMapper.h"
#include "base/exceptions.h"
#include "interfaces/IOutputEngine.h"
#include "json-schema.hpp"
#include "plugins/engines/adios/AdiosEngineImpl.h"
using nlohmann::json;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {

class AdiosEngineImpl {
  int rank;
  int size;
  int engineStep;
  MPI_Comm comm;

  adios2::IO& io; /**< @todo  */
  adios2::Engine engine;
  adios2::Engine rootEngine;
  bool root;

  bool localmode = false;



 public:

  static void AdiosEngineImpl::Declare(adios2::IO& io, std::set<std::string> exists);


    AdiosEngineImpl(adios2::IO& io_, std::string filename, std::string metafname, MPI_Comm comm, long uid, int rootRank = 0);

  ~AdiosEngineImpl();

  void Put(std::string variableName, IDataType_ptr ptr, const MetaData& m,
           IOutputEngine* oengine, int writeRank);

  template <typename T>
  void AdiosEngineImpl::Put(std::string variableName, const T& value,
                            const MetaData& m, int writerank) {
    if (root) {
      rootEngine.BeginStep();
      rootEngine.Put("name", variableName);
      rootEngine.Put("metaData", StringUtils::metaDataToJsonString(m));
      rootEngine.Put("stage", engineStep);
      rootEngine.Put("output", writerank);
    }

    if (rank == writerank) {
      engine.BeginStep();
      engine.Put("stage", engineStep);
      Put(value, false);
      engine.EndStep();
    } else if (writerank < 0) {
      engine.BeginStep();
      engine.Put("stage", engineStep);
      Put(value, true);
      engine.EndStep();
    }
    if (root) {
      rootEngine.EndStep();
    }

    ++engineStep;
  }

  void Put(const double& value, bool local);
  void Put(const long long& value, bool local);
  void Put(const std::string& value, bool local);
  void Put(const json& value, bool local);
  void Put(const bool& value, bool local);

  void PutGlobalArray(long long dtype, std::string variableName,
                      IDataType_vec data, std::vector<size_t> gsizes,
                      std::vector<std::size_t> sizes, std::vector<std::size_t> offset,
                      const MetaData& m) override;

  void Log(long uid, const char* package, int stage, std::string level,
           std::string message, int rank = 0);

  void finalize();

  void injectionPointEndedCallBack(std::string id, InjectionPointType type_,
                                   std::string stageId);

  void injectionPointStartedCallBack(long uid, std::string packageName,
                                     std::string id, InjectionPointType type_,
                                     std::string stageId);

  void testStartedCallBack(std::string packageName, std::string testName,
                           bool internal);

  void testFinishedCallBack(bool result_);

  void unitTestStartedCallBack(std::string packageName,
                               std::string unitTestName);

  void unitTestFinishedCallBack(IUnitTest* tester);

  void dataTypeStartedCallBack(std::string variableName, long long dtype,
                               const MetaData& m);

  void dataTypeEndedCallBack(std::string variableName);
};

}  // namespace Engines

}  // namespace VNVPACKAGENAME

}  // namespace VnV

#endif  // AdiosEngineImpl_H
