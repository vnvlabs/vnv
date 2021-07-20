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

enum class AdiosDataType {
  JSON = 0,
  STRING = 1,
  DOUBLE = 2,
  BOOL = 3,
  LONGLONG = 4,
  DATA_TYPE_START = 5,
  DATA_TYPE_END = 6,
  TEST_START = 7,
  TEST_END = 8,
  VECTOR_START = 9,
  VECTOR_END = 10,
  INJECTION_POINT_BEGIN_BEGIN = 11,
  INJECTION_POINT_BEGIN_END = 12,
  INJECTION_POINT_ITER_BEGIN = 13,
  INJECTION_POINT_ITER_END = 14,
  INJECTION_POINT_END_BEGIN = 15,
  INJECTION_POINT_END_END = 16,
  UNIT_TEST_START = 17,
  UNIT_TEST_END = 18,
  INFO = 19,
  COMM_MAP=20,
  LOG=21,
  INJECTION_POINT_SINGLE_BEGIN = 22,
  INJECTION_POINT_SINGLE_END = 23
};


class AdiosEngineImpl {
  int rank;
  int size;
  int engineStep;
  MPI_Comm comm;

  adios2::IO& io; /**< @todo  */
  adios2::Engine engine;
  bool root;

 public:

    AdiosEngineImpl(adios2::IO& io_, const std::string& filename, MPI_Comm comm, long uid, int rootRank = 0);

  ~AdiosEngineImpl();

  void type(AdiosDataType type);

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

  void injectionPointEndedCallBack(long long nextId, const std::string& id, InjectionPointType type_,
                                   const std::string& stageId);

  void injectionPointStartedCallBack(long uid, const std::string& packageName,
                                     const std::string& id, InjectionPointType type_,
                                     const std::string& stageId);

  void testStartedCallBack(const std::string& packageName, const std::string& testName,
                           bool internal, long uid);

  void testFinishedCallBack(bool result_);

  void unitTestStartedCallBack(const std::string& packageName,
                               const std::string& unitTestName);

  void unitTestFinishedCallBack(IUnitTest* tester);

  static long ADIOS_ENGINE_IMPL_VERSION;
  void writeInfo();

  static void Define(adios2::IO& io);
};

}  // namespace Engines

}  // namespace VNVPACKAGENAME

}  // namespace VnV

#endif  // AdiosEngineImpl_H
