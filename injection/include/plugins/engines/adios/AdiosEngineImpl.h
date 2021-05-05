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
#include "base/Utilities.h"
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
  bool root;

 public:

    AdiosEngineImpl(adios2::IO& io_, const std::string& filename, const std::string& metafname, MPI_Comm comm, long uid, int rootRank = 0);

  ~AdiosEngineImpl();

  void Put(const std::string& variableName, const IDataType_ptr& ptr, const MetaData& m,
           IOutputEngine* oengine, int writeRank);

  template <typename T>
  void Put(std::string variableName, const T& value,
                            const MetaData& m, int writerank) {
    engine.BeginStep();
    if (root) {
      engine.Put("name", variableName);
      engine.Put("metaData", StringUtils::metaDataToJsonString(m));
      engine.Put("output", writerank);
    }

    if (rank == writerank) {
      Put(value, false);
    } else if (writerank < 0) {
      Put(value, true);
    }
    engine.EndStep();
  }

  void Put(const double& value, bool local);
  void Put(const long long& value, bool local);
  void Put(const std::string& value, bool local);
  void Put(const json& value, bool local);
  void Put(const bool& value, bool local);

  void PutGlobalArray(long long dtype, const std::string& variableName,
                      const IDataType_vec& data, std::vector<size_t> gsizes,
                      std::vector<std::size_t> sizes, std::vector<std::size_t> offset,
                      const MetaData& m) ;

  void Log(long uid, const char* package, int stage, const std::string& level,
           const std::string& message, int rank = 0);

  void finalize();

  void injectionPointEndedCallBack(const std::string& id, InjectionPointType type_,
                                   const std::string& stageId);

  void injectionPointStartedCallBack(long uid, const std::string& packageName,
                                     const std::string& id, InjectionPointType type_,
                                     const std::string& stageId);

  void testStartedCallBack(const std::string& packageName, const std::string& testName,
                           bool internal);

  void testFinishedCallBack(bool result_);

  void unitTestStartedCallBack(const std::string& packageName,
                               const std::string& unitTestName);

  void unitTestFinishedCallBack(IUnitTest* tester);

  static long ADIOS_ENGINE_IMPL_VERSION;
  void writeInfo();

  void writeCommMap(const json& commMap);

  static void Define(adios2::IO& io);
};

}  // namespace Engines

}  // namespace VNVPACKAGENAME

}  // namespace VnV

#endif  // AdiosEngineImpl_H
