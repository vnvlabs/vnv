#ifndef IOUTPUTENGINE_H
#define IOUTPUTENGINE_H

#include <string>

#include "base/CommunicationStore.h"
#include "base/exceptions.h"
#include "c-interfaces/Communication.h"
#include "c-interfaces/Logging.h"
#include "c-interfaces/PackageName.h"
#include "c-interfaces/Wrappers.h"
#include "interfaces/IUnitTest.h"
#include "interfaces/nodes/Nodes.h"
#include "json-schema.hpp"
#include "base/Communication.h"

/**
 * @brief The IOutputEngine class
 */
using nlohmann::json;

namespace VnV {

using namespace Communication;

enum class InjectionPointType { Single, Begin, End, Iter };
namespace InjectionPointTypeUtils {

std::string getType(InjectionPointType type, std::string stageId);
int toC(InjectionPointType type);
}  // namespace InjectionPointTypeUtils

enum class VariableEnum { Double, String, Int, Float, Long };
namespace VariableEnumFactory {
VariableEnum fromString(std::string s);
std::string toString(VariableEnum e);
}  // namespace VariableEnumFactory


class BaseAction {
public:
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const  = 0;
  virtual int count(ICommunicator_ptr comm, int engineRoot) const = 0;
};



class IOutputEngine {
 protected:
  virtual void dataTypeStartedCallBack(ICommunicator_ptr comm, std::string variableName,
                                       long long dtype) = 0;
  virtual void dataTypeEndedCallBack(ICommunicator_ptr comm,
                                     std::string variableName) = 0;

 public:
  virtual int getRoot(ICommunicator_ptr ptr) { return 0; }

  virtual void Put(std::string variableName,
                   const bool& value) = 0;

  virtual void Put(std::string variableName,
                   const int &value) = 0;

  virtual void Put(std::string variableName,
                   const long &value) = 0;

  virtual void Put(std::string variableName,
                   const double& value) = 0;

  virtual void Put(std::string variableName,
                   const json& value) = 0;

  virtual void Put(std::string variableName,
                   const std::string& value) = 0;


  virtual void Log(ICommunicator_ptr comm, const char* packageName, int stage,
                   std::string level, std::string message) = 0;

  /**
   * @brief PutGlobalArray
   * @param comm: The communicator that will be used
   * @param dtype: The datatype of the data
   * @param variableName: The name that the variable should be stored in
   * @param data: The local data on the machine
   * @param gsizes: The global shape of the array
   * @param sizes: The local shape of the local data array prod(sizes) = len(data)
   * @param offset The local offset into the global array.
   * @param onlyOne If >=0 this parameter indicates that only One process has information about the node.
   */
  virtual void PutGlobalArray(ICommunicator_ptr comm ,
                              long long dtype,
                              std::string variableName,
                              IDataType_vec data,
                              std::vector<int> gsizes,
                              std::vector<int> sizes,
                              std::vector<int> offset,
                              int onlyOne=-1 ) = 0;


  template <typename T>
  void Write(ICommunicator_ptr comm, std::string variableName, T* data, const BaseAction& action) {

    int total = action.count(comm, getRoot(comm));

    IDataType_vec vec(total);

    auto &c = CommunicationStore::instance();
    auto it = c.getDataType(typeid(T).name());
    if (it != nullptr ) {
      long long key = it->getKey();
      for (int i = 0; i < total; i++) {
          vec.push_back( c.getDataType(key) );
          vec.back()->setData((void*) &(data[i]));
      }
    } else {
       throw VnVExceptionBase("DataType unknown");
    }
    action.write(comm, it->getKey(), variableName, vec, this);
  }

  virtual ~IOutputEngine() = default;
};
// Action to write <size> values from process <rank> to the engine.
// If r < 0 then the value on the engines root is used.
class ScalarAction : public BaseAction {
public:

  int rank;
  int size;
  ScalarAction(int s, int r = -1) : rank(r), size(s) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    std::vector<int> val(1);
    val[0] = ( (rank < 0 && comm->Rank() == engine->getRoot(comm))  || (comm->Rank() == rank)  ) ? size : 0 ;
    engine->PutGlobalArray(comm, dtype, variableName,data, {size} , val , {0}, rank );
  }
  virtual int count(ICommunicator_ptr comm, int engineRoot) const override {
    return ( (rank < 0 && comm->Rank() == engineRoot ) || (comm->Rank() == rank)  ) ? size : 0 ;
  };

};

class SingleScalarAction : public ScalarAction {
public:
  SingleScalarAction(int r = -1) : ScalarAction(1,r) {}
};

//Write a global vector with <size> elements off every process. This is
// indexed by the rank.
class VectorAction : public BaseAction {
public:
  int size;
  VectorAction(int s = 1) : size(s) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    engine->PutGlobalArray(comm, dtype, variableName,data, {size*comm->Size()}, {size} , {size*comm->Rank()});
  }
  virtual int count(ICommunicator_ptr ptr, int engineRoot) const override {
    return size;
  };
};

// Take a [x,y] matrix on each processor and turn it into a [a*x,y*ymax] matrix indexed by the processor.
// An error will be thrown when ymax is not a factor of the comm->Size ().
class MatrixAction : public BaseAction {
public:
  int x, y, ymax;
  MatrixAction(int x_, int y_, int columns = 1) : x(x_), y(y_), ymax(columns) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    int s = comm->Size();
    if (s % ymax != 0) throw VnV::VnVExceptionBase("Invalid matrix Size given for Matrix Action");

    int r = comm->Rank();
    int xs = x * comm->Size() / ymax;
    int ys = y * ymax ;
    int xoff = r / ymax ;
    int yoff = r % ymax ;
    engine->PutGlobalArray(comm, dtype, variableName,data, {xs,ys}, {x,y} , {xoff,yoff});
  }
  virtual int count(ICommunicator_ptr ptr, int engineRoot) const override {
    return x*y ;
  };
};

// GlobalArrayAction --> The most generic action --> Just put a global array
// with user supplied global size, local size and offsets.
class GlobalArrayAction : public BaseAction {
public:
  std::vector<int> gsize,lsize,offsets;
  GlobalArrayAction(std::vector<int> &gsizes, std::vector<int> &lsizes, std::vector<int> &offs) : gsize(gsizes), lsize(lsizes), offsets(offs) {
  }
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    engine->PutGlobalArray(comm, dtype, variableName,data, gsize, lsize, offsets);
  }
  virtual int count(ICommunicator_ptr ptr, int engineRoot) const override {
    return std::accumulate(lsize.begin(), lsize.end(), 1, std::multiplies<int>());
  };

};

//Reduce a global vector. Here reducer is the name of some VnV::IReduction registered
// with the CommunicationStore. <s> is the size of the vector on the "LOCAL" processor.
// r is the process to reduce down to.
// NOTE: In this case we assume all elements on the local process are part of some global
// vector. So, calling this operation reduces across that vector. This is different to the
// standard MPI_Reduce functions, which do an element wise reduction. For ElementWise reduction
// use ElementWiseReductionAction instead.
class ReductionAction : public BaseAction {
public:
  IReduction_ptr reducer;
  int size, root;
  ReductionAction(std::string red, int s = 1, int r = -1) : size(s), root(r) {
    reducer = CommunicationStore::instance().getReducer(red);
  }
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    Communication::DataTypeCommunication d(comm);

    int rank = (root < 0 ) ? engine->getRoot(comm) : root ; // r<0 uses the engine root as root to save communication .

    IDataType_ptr result = d.ReduceVector(data, dtype, reducer, rank);
    std::vector<int> r(1);
    r[0] = (comm->Rank() == rank ) ? 1 : 0;
    engine->PutGlobalArray(comm, dtype, variableName, {result}, {1}, r , {0}, rank);
  }
  virtual int count(ICommunicator_ptr ptr, int engineRoot) const override {
    return size;
  };
};

//Reduce a global vector. Here reducer is the name of some VnV::IReduction registered
// with the CommunicationStore. <s> is the size of the vector on the "LOCAL" processor.
// r is the process to reduce down to.
// NOTE: In this case we do an element wise reduction. ALL PROCESSORS MUST SET SIZE TO
// BE THE SAME OR THIS WILL BREAK SPECTACULARLY

class ElementWiseReductionAction : public BaseAction {
public:
  IReduction_ptr reducer;
  int size, root;
  ElementWiseReductionAction(std::string red, int s = 1, int r = -1) : size(s), root(r) {
    reducer = CommunicationStore::instance().getReducer(red);
  }
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    int rank =  (root < 0 ) ? engine->getRoot(comm) : root ; // r<0 uses the engine root as root to save communication .
    Communication::DataTypeCommunication d(comm);
    IDataType_vec result = d.ReduceMultiple(data, dtype ,reducer, rank);
    std::vector<int> offs(size,0);
    std::vector<int> lsize(1);
    lsize[0] = (comm->Rank() == rank) ? 1 : 0 ;
    engine->PutGlobalArray(comm, dtype, variableName,result, {size} ,  lsize ,  offs, rank);
  }
  virtual int count(ICommunicator_ptr ptr, int engineRoot) const override {
    return size;
  };
};

// Reduce the vector on the local processor and write one value per processor.
// If size is one, this is the same as Scalar Action.
class ProcessorWiseReductionAction : public ElementWiseReductionAction {
public:
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const  override {
      Communication::DataTypeCommunication d(comm);
      IDataType_ptr result = d.ReduceLocalVec(data, reducer);
      engine->PutGlobalArray(comm, dtype, variableName , {result}, {comm->Size()}, {1},{comm->Rank()});
  }
};

class SingleProcessorReductionAction : public ProcessorWiseReductionAction {
public:
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data, IOutputEngine *engine) const override {
    int rank =  (root < 0 ) ? engine->getRoot(comm) : root ; // r<0 uses the engine root as root to save communication .
    Communication::DataTypeCommunication d(comm);
    IDataType_vec result;
    if ( comm->Rank() == rank) {
        result = d.ReduceMultiple(data, dtype ,reducer, rank);
    }
        std::vector<int> offs(size,0);
    std::vector<int> lsize(1);
    lsize[0] = (comm->Rank() == rank) ? 1 : 0 ;
    engine->PutGlobalArray(comm, dtype, variableName,result, {size} ,  lsize ,  offs, rank);
  }

  virtual int count(ICommunicator_ptr comm, int engineRoot) const override {
    int rank =  (root < 0 ) ? engineRoot : root ; // r<0 uses the engine root as root to save communication .
    return (comm->Rank() == rank) ? size : 0 ;
  }

};

class IInternalOutputEngine : public IOutputEngine {
 public:
  virtual void setFromJson(json& configuration) = 0;
  virtual json getConfigurationSchema() = 0;
  virtual void injectionPointStartedCallBack(ICommunicator_ptr comm,
                                             std::string packageName,
                                             std::string id,
                                             InjectionPointType type,
                                             std::string stageId) = 0;
  virtual void injectionPointEndedCallBack(ICommunicator_ptr comm, std::string id,
                                           InjectionPointType type,
                                           std::string stageId) = 0;
  virtual void testStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                                   std::string testName, bool internal) = 0;
  virtual void testFinishedCallBack(ICommunicator_ptr comm, bool result_) = 0;
  virtual void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName,
                                       std::string unitTestName) = 0;
  virtual void unitTestFinishedCallBack(ICommunicator_ptr comm, IUnitTest* tester) = 0;

  virtual Nodes::IRootNode* readFromFile(std::string file, long& idCounter) = 0;
  virtual std::string print() = 0;
  virtual void finalize() = 0;
  virtual ~IInternalOutputEngine() = default;
};

/**
 * @brief The OutputEngineManager class
 */
class OutputEngineManager : public IInternalOutputEngine {
 public:
  void set(json& configuration);
  IOutputEngine* getOutputEngine();
  virtual ~OutputEngineManager() = default;
};

typedef OutputEngineManager* engine_register_ptr();

void registerEngine(std::string name, VnV::engine_register_ptr r);

}  // namespace VnV

#define INJECTION_ENGINE(PNAME, name)                               \
  namespace VnV {                                                   \
  namespace PNAME {                                                 \
  namespace Engines {                                               \
  OutputEngineManager* declare_##name();                            \
  void register_##name() { registerEngine(#name, declare_##name); } \
  }                                                                 \
  }                                                                 \
  }                                                                 \
  VnV::OutputEngineManager* VnV::PNAME::Engines::declare_##name()

#define DECLAREENGINE(PNAME, name) \
  namespace VnV {                  \
  namespace PNAME {                \
  namespace Engines {              \
  void register_##name();          \
  }                                \
  }                                \
  }
#define REGISTERENGINE(PNAME, name) VnV::PNAME::Engines::register_##name();

#endif  // IOUTPUTENGINE_H
