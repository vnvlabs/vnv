#ifndef IOUTPUTENGINE_H
#define IOUTPUTENGINE_H

#include <string>
#include <type_traits>

#include "base/InjectionPointConfig.h"
#include "base/Communication.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/DataTypeStore.h"
#include "base/stores/ReductionStore.h"
#include "c-interfaces/Wrappers.h"
#include "common-interfaces/Communication.h"
#include "common-interfaces/Logging.h"
#include "common-interfaces/PackageName.h"
#include "interfaces/IUnitTest.h"
#include "validate/json-schema.hpp"
#define FetchTypes X(std::string) X(int) X(double) X(long)



/**
 * @brief The IOutputEngine class
 */
using nlohmann::json;

namespace VnV {

class IOutputEngine;

namespace InjectionPointTypeUtils {

std::string getType(InjectionPointType type, std::string stageId);
int toC(InjectionPointType type);
}  // namespace InjectionPointTypeUtils

class BaseAction {
 public:
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const = 0;
  virtual int count(ICommunicator_ptr comm, int engineRoot) const = 0;

  template <typename T> std::vector<T> flatten(std::vector<std::vector<T>>& data) {
    if (data.size() == 0) return {};

    int rows = data.size();
    int cols = data[0].size();

    std::vector<T> res;
    res.reserve(rows * cols);

    for (int i = 0; i < rows; i++) {
      auto& col = data[i];
      for (int j = 0; j < col.size(); j++) {
        res.push_back(col[j]);
      }
    }
    return res;
  }
};

class IOutputEngine {
 protected:
  ICommunicator_ptr comm;

 public:
  virtual void setCommunicator(ICommunicator_ptr ptr) { comm = ptr; }

  virtual int getRoot() { return 0; }

  virtual void Log(ICommunicator_ptr comm, const char* packageName, int stage, std::string level,
                   std::string message) = 0;

  virtual bool Fetch(std::string /* message */, const json& /* schema */, long /* timeoutInSeconds */,
                     json& /* response */) {
    return false;
  }

  virtual void Put(std::string variableName, const bool& value, const MetaData& m = MetaData()) = 0;

  virtual void Put(std::string variableName, const long long& value, const MetaData& m = MetaData()) = 0;

  virtual void Put(std::string variableName, const double& value, const MetaData& m = MetaData()) = 0;

  virtual void Put(std::string variableName, const json& value, const MetaData& m = MetaData()) = 0;

  virtual void Put(std::string variableName,

                   const std::string& value, const MetaData& m = MetaData()) = 0;

  virtual void Put(std::string variableName, IDataType_ptr data, const MetaData& m = MetaData()) = 0;

  virtual void Put(std::string variableName, const char* value, const MetaData& m = MetaData()) {
    std::string s(value);
    Put(variableName, s, m);
  }

  // Get all the integral types and feed them to long long.
  template <typename T, typename std::enable_if<std::is_integral<T>::value, T>::type* = nullptr>
  void Put(std::string variableName, const T& value, const MetaData& m = MetaData()) {
    long long b = value;
    this->Put(variableName, b, m);
  }

  // Get all the floating point types and feed them to double.
  template <typename T, typename std::enable_if<std::is_floating_point<T>::value, T>::type* = nullptr>
  void Put(std::string variableName, const T& value, const MetaData& m = MetaData()) {
    double b = value;
    this->Put(variableName, b, m);
  }

  template <typename T, typename = void> struct vnv_has_datatype : std::false_type {};

  template <typename T> struct vnv_has_datatype<T, decltype((void)T::vnv_datatype, void())> : std::true_type {};

  // Get all the  class types that might have a datatype wrapper.
  template <typename T, typename std::enable_if<vnv_has_datatype<T>{}, T>::type* = nullptr>
  void Put(std::string variableName, const T& value, const MetaData& m = MetaData()) {
    try {
      auto it = DataTypeStore::instance().getDataType(T::vnv_datatype);
      if (it != nullptr) {
        it->setData(&value);
        this->Put(variableName, it, m);
      }
    } catch (...) {
      VnV_Error(VNVPACKAGENAME, "Could not put Data Type %s ", T::vnv_datatype.c_str());
    }
    VnV_Error(VNVPACKAGENAME, "Could not put Data Type %s because it does not exist ", T::vnv_datatype.c_str());
  }

  template <typename T>
  void Write(std::string variableName, long long key, T* data, const BaseAction& action,
             const MetaData& m = MetaData()) {
    int total = action.count(comm, getRoot());

    try {
      IDataType_vec vec(total);
      auto& c = DataTypeStore::instance();

      for (int i = 0; i < total; i++) {
        auto di = c.getDataType(key);
        if (di != nullptr) {
          di->setData((void*)&(data[i]));
          vec[i] = di;
        } else {
          VnV_Error(VNVPACKAGENAME, "Could not write %s as datatype does not exist", variableName.c_str());
          return;
        }
      }
      action.write(comm, key, variableName, vec, this, m);

    } catch (...) {
      VnV_Error(VNVPACKAGENAME, "Could not write %s", variableName.c_str());
    }
  }

  template <typename T, typename std::enable_if<vnv_has_datatype<T>{}, T>::type* = nullptr>
  void Write(std::string variableName, T* data, const BaseAction& action, const MetaData& m = MetaData()) {
    auto did = DataTypeStore::instance().getDataType(T::vnv_datatype);
    if (did != nullptr) {
      Write(variableName, did->getKey(), data, action, m);
    } else {
      VnV_Error(VNVPACKAGENAME, "Could not write %s", variableName.c_str());
    }
  }

  template <typename T, typename std::enable_if<!(vnv_has_datatype<T>{}), T>::type* = nullptr>
  void Write(std::string variableName, T* data, const BaseAction& action, const MetaData& m = MetaData()) {
    auto did = DataTypeStore::instance().getDataType(typeid(T).name());
    if (did != nullptr) {
      Write(variableName, did->getKey(), data, action, m);
    } else {
      VnV_Error(VNVPACKAGENAME, "Could not write %s", variableName.c_str());
    }
  }

  /**
   * @brief PutGlobalArray
   * @param comm: The communicator that will be used
   * @param dtype: The datatype of the data
   * @param variableName: The name that the variable should be stored in
   * @param data: The local data on the machine
   * @param gsizes: The global shape of the array
   * @param sizes: The local shape of the local data array prod(sizes) =
   * len(data)
   * @param offset The local offset into the global array.
   */
  virtual void PutGlobalArray(long long dtype, std::string variableName, IDataType_vec data, std::vector<int> gsizes,
                              std::vector<int> sizes, std::vector<int> offset, const MetaData& m) = 0;

  // Write a vector of size using data only on rank r!
  template <typename T>
  void Put_Rank(std::string variableName, std::vector<T>& data, int rank = -1, const MetaData& m = MetaData());

  template <typename T>
  void Put_Rank(std::string variableName, int size, T* data, int rank = -1, const MetaData& m = MetaData());

  // Write the value on rank r only!
  template <typename T> void Put_Rank(std::string variableName, T& data, int rank = -1, const MetaData& m = MetaData());

  // Write a global vector. This assumes vector size is same on all procs.
  // Values are indexed by rank!
  template <typename T> void Put_Vector(std::string variableName, std::vector<T>& data, const MetaData& m = MetaData());

  // Write a global vector. This assumes vector size is same on all procs.
  // Values are indexed by rank!
  template <typename T> void Put_Vector(std::string variableName, int size, T* data, const MetaData& m = MetaData());

  // Write a global vector -- one element per rank. This assumes vector size is
  // same on all procs. Values are indexed by rank!
  template <typename T> void Put_Vector(std::string variableName, T& data, const MetaData& m = MetaData());

  // Take a [x,y] matrix on each processor and turn it into a [a*x,y*cols]
  // matrix block indexed by the processor.
  //  An error will be thrown when cols is not a factor of the comm->Size().
  template <typename T>
  void Put_Matrix(std::string variableName, std::vector<std::vector<T>>& data, int cols,
                  const MetaData& m = MetaData());

  template <typename T>
  void Put_Matrix(std::string variableName, int xdim, std::vector<T>& data, int cols, const MetaData& m = MetaData());

  template <typename T>
  void Put_Matrix(std::string variableName, int xdim, int ydim, T* data, int cols, const MetaData& m = MetaData());

  // Take a generic matrix with offsets. You should pass in the global size and
  // the offsets. We assume the size of the data is accurate such that all data
  // should be sent.
  template <typename T>
  void Put_Matrix(std::string variableName, std::vector<std::vector<T>>& data, std::pair<int, int>& gsize,
                  std::pair<int, int>& offsets, const MetaData& m = MetaData());

  // Flat version of above for matrices that are allready flat.
  template <typename T>
  void Put_Matrix(std::string variableName, int xdim, std::vector<T>& data, std::pair<int, int>& gsize,
                  std::pair<int, int>& offsets, const MetaData& m = MetaData());

  template <typename T>
  void Put_Matrix(std::string variableName, T* data, std::pair<int, int>& lsize, std::pair<int, int>& gsize,
                  std::pair<int, int>& offsets, const MetaData& m = MetaData());

  // Apply a reduction across a global reducer. Here reducer is the name of some
  // VnV::IReducer interface. root is the process to reduce the data too. root<0
  // means the toolkit can decide the root. This is usually best as we can set
  // the root process equal to the root process of the IOutputEngine.
  template <typename T>
  void Put_ReduceVector(std::string variableName, std::string reducer, std::vector<T>& data, int root = -1,
                        const MetaData& m = MetaData());

  template <typename T>
  void Put_ReduceVector(std::string variableName, std::string reducer, int size, T* data, int root = -1,
                        const MetaData& m = MetaData());

  // Apply a reduction across a scalar. Here reducer is the name of some
  // VnV::IReducer interface. root is the process to reduce the data too.
  template <typename T>
  void Put_ReduceScalar(std::string variableName, std::string reducer, T& data, int root = -1,
                        const MetaData& m = MetaData());

  // Apply a reduction across a global reducer. Here reducer is the name of some
  // VnV::IReducer interface. root is the process to reduce the data too. This
  // reduces the vector element wise. So, in the end you get a vector the same
  // size as data. This is similar to how MPI_Reduce works. Put_ReduceVector
  // does a local reduction on the local vector and then performs an elementwise
  // reduction on the result (a vector of length 1).
  template <typename T>
  void Put_ReduceVectorElementWise(std::string variableName, std::string reducer, std::vector<T>& data, int root = -1,
                                   const MetaData& m = MetaData());
  template <typename T>
  void Put_ReduceVectorElementWise(std::string variableName, std::string reducer, int size, T* data, int root = -1,
                                   const MetaData& m = MetaData());

  // This one reduces the vector processor wise. You end up with a vector of
  // length = MPI_COMM_SIZE
  //  where each element v_r is the result of the reduction applied to the local
  //  vector on rank r. This one does a local reduction then calls
  //  PutGlobalArray (rather than allReducing to the root and writing a local
  //  array.
  template <typename T>
  void Put_ReduceVectorRankWise(std::string variableName, std::string reducer, std::vector<T>& data, int root = -1,
                                const MetaData& m = MetaData());

  template <typename T>
  void Put_ReduceVectorRankWise(std::string variableName, std::string reducer, int size, T* data, int root = -1,
                                const MetaData& m = MetaData());

  // This one reduces the vector on rank == root and writes the result as a
  // scalar value. This is
  //  a local reduction on a single core. Use this one if the data has already
  //  been reduced to a single processor for some other reason.
  template <typename T>
  void Put_ReduceVectorRankOnly(std::string variableName, std::string reducer, std::vector<T>& data, int root = -1,
                                const MetaData& m = MetaData());

  template <typename T>
  void Put_ReduceVectorRankOnly(std::string variableName, std::string reducer, int size, T* data, int root = -1,
                                const MetaData& m = MetaData());

  virtual ~IOutputEngine() = default;
};
// Action to write <size> values from process <rank> to the engine.
// If r < 0 then the value on the engines root is used.
class ScalarAction : public BaseAction {
 public:
  int rank;
  int size;
  ScalarAction(int s, int r = -1) : rank(r), size(s) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    std::vector<int> val(1);
    val[0] = ((rank < 0 && comm->Rank() == engine->getRoot()) || (comm->Rank() == rank)) ? size : 0;
    engine->PutGlobalArray(dtype, variableName, data, {size}, val, {0}, m);
  }

  virtual int count(ICommunicator_ptr comm, int engineRoot) const override {
    return ((rank < 0 && comm->Rank() == engineRoot) || (comm->Rank() == rank)) ? size : 0;
  };
};

// Write a vector of size using data only on rank r!
template <typename T>
void IOutputEngine::Put_Rank(std::string variableName, std::vector<T>& data, int rank, const MetaData& m) {
  Put_Rank(variableName, data.size(), data.data(), rank, m);
}
template <typename T>
void IOutputEngine::Put_Rank(std::string variableName, int size, T* data, int rank, const MetaData& m) {
  ScalarAction action(size, rank);
  Write(variableName, data, action, m);
}

// Write one value from rank r only!
template <typename T> void IOutputEngine::Put_Rank(std::string variableName, T& data, int rank, const MetaData& m) {
  Put_Rank(variableName, 1, &data, rank, m);
}

// Write a global vector with <size> elements off every process. This is
//  indexed by the rank.
class VectorAction : public BaseAction {
 public:
  int size;
  VectorAction(int s = 1) : size(s) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    engine->PutGlobalArray(dtype, variableName, data, {size * comm->Size()}, {size}, {size * comm->Rank()}, m);
  }
  virtual int count(ICommunicator_ptr /* ptr */, int /* engineRoot */) const override { return size; };
};

// Write a global vector. This assumes vector size is same on all procs. Values
// are indexed by rank!
template <typename T> void IOutputEngine::Put_Vector(std::string variableName, int size, T* data, const MetaData& m) {
  VectorAction action(size);
  Write(variableName, data, action, m);
}

// Write a global vector. This assumes vector size is same on all procs. Values
// are indexed by rank!
template <typename T>
void IOutputEngine::Put_Vector(std::string variableName, std::vector<T>& data, const MetaData& m) {
  Put_Vector(variableName, data.size(), data.data(), m);
}

// Write a global vector -- one element per rank. This assumes vector size is
// same on all procs. Values are indexed by rank!
template <typename T> void IOutputEngine::Put_Vector(std::string variableName, T& data, const MetaData& m) {
  Put_Vector(variableName, 1, &data, m);
}

// Take a [x,y] matrix on each processor and turn it into a [a*x,y*ymax] matrix
// indexed by the processor. An error will be thrown when ymax is not a factor
// of the comm->Size ().
class MatrixAction : public BaseAction {
 public:
  int x, y, ymax;
  MatrixAction(int x_, int y_, int columns = 1) : x(x_), y(y_), ymax(columns) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    int s = comm->Size();  // number of processors.

    // If ymax is not a multiple of y we have an issue
    // If ymax/y is not a multiple of s we have an issue
    if ((ymax % y != 0) && (s % (ymax / y) != 0)) {
      VnV_Error(VNVPACKAGENAME,
                "Invalid matrix Size given for Matrix Action. ymax should"
                "be a multiple of y and (ymax/y) should be a multiple of s."
                "{ ymax: %d, y: %d , s: %d,  ",
                ymax, y, s);
      return;
    }

    int r = comm->Rank();
    int xs = x * y * comm->Size() / ymax;
    int ys = ymax;
    int xoff = (r * x * y) / ymax;
    int yoff = (r * y) % ymax;
    engine->PutGlobalArray(dtype, variableName, data, {xs, ys}, {x, y}, {xoff, yoff}, m);
  }
  virtual int count(ICommunicator_ptr /* ptr */, int /* engineRoot */) const override { return x * y; };
};

template <typename T>
void IOutputEngine::Put_Matrix(std::string variableName, int x, int y, T* data, int cols, const MetaData& m) {
  MatrixAction action(x, y, cols);
  Write(variableName, data, action, m);
}

template <typename T>
void IOutputEngine::Put_Matrix(std::string variableName, std::vector<std::vector<T>>& data, int cols,
                               const MetaData& m) {
  if (data.size() == 0) {
    return;
  }
  int ydim = data[0].size();
  MatrixAction action(data.size(), ydim, cols);
  std::vector<T> flatData = action.flatten(data);
  Write(variableName, flatData.data(), action, m);
}

template <typename T>
void IOutputEngine::Put_Matrix(std::string variableName, int x, std::vector<T>& data, int cols, const MetaData& m) {
  Put_Matrix(variableName, x, data.size() / x, data.data(), cols, m);
}

// GlobalArrayAction --> The most generic action --> Just put a global array
// with user supplied global size, local size and offsets.
class GlobalArrayAction : public BaseAction {
 public:
  std::vector<int> gsize, lsize, offsets;
  GlobalArrayAction(std::vector<int>& gsizes, std::vector<int>& lsizes, std::vector<int>& offs)
      : gsize(gsizes), lsize(lsizes), offsets(offs) {}

  virtual void write(ICommunicator_ptr /* comm */, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    engine->PutGlobalArray(dtype, variableName, data, gsize, lsize, offsets, m);
  }
  virtual int count(ICommunicator_ptr /* ptr */, int /* engineRoot */) const override {
    return std::accumulate(lsize.begin(), lsize.end(), 1, std::multiplies<int>());
  }
};

// Take a generic matrix with offsets. You should pass in the global size and
// the offsets. We assume the size of the data is accurate such that all data
// should be sent.
template <typename T>
void IOutputEngine::IOutputEngine::Put_Matrix(std::string variableName, std::vector<std::vector<T>>& data,
                                              std::pair<int, int>& gsize, std::pair<int, int>& offsets,
                                              const MetaData& m) {
  if (data.size() == 0) {
    return;
  }
  int dsize = data.size();
  int ddsize = data[0].size();
  std::vector<int> lsizes = {dsize, ddsize};
  std::vector<int> gsizes = {gsize.first, gsize.second};
  std::vector<int> offs = {offsets.first, offsets.second};

  GlobalArrayAction action(gsizes, lsizes, offs);
  std::vector<T> flatData = action.flatten(data);
  Write(variableName, flatData.data(), action, m);
}

// Take a generic matrix with offsets. You should pass in the global size and
// the offsets. We assume the size of the data is accurate such that all data
// should be sent. Xdim is the x dimension of the local matrix -- we figure out
// the ydim
//  from there.

template <typename T>
void IOutputEngine::IOutputEngine::Put_Matrix(std::string variableName, T* data, std::pair<int, int>& lsize,
                                              std::pair<int, int>& gsize, std::pair<int, int>& offsets,
                                              const MetaData& m) {
  std::vector<int> lsizes = {lsize.first, lsize.second};
  std::vector<int> gsizes = {gsize.first, gsize.second};
  std::vector<int> offs = {offsets.first, offsets.second};

  GlobalArrayAction action(gsizes, lsizes, offs);
  Write(variableName, data, action, m);
}

template <typename T>
void IOutputEngine::IOutputEngine::Put_Matrix(std::string variableName, int xdim, std::vector<T>& data,
                                              std::pair<int, int>& gsize, std::pair<int, int>& offsets,
                                              const MetaData& m) {
  if (data.size() == 0) {
    return;
  }
  if (data.size() % xdim != 0) throw "Invalid Data Size passed to Put Matrix";

  std::vector<int> gsizes = {gsize.first, gsize.second};
  std::vector<int> offs = {offsets.first, offsets.second};
  std::vector<int> lsizes = {xdim, data.size() / xdim};

  GlobalArrayAction action(gsizes, lsizes, offs);
  Write(variableName, data.data(), action, m);
}

// Take a generic matrix with offsets. You should pass in the global size and
// the offsets. We assume the size of the data is accurate such that all data
// should be sent. Xdim is the x dimension of the local matrix -- we figure out
// the ydim
//  from there.

// Reduce a global vector. Here reducer is the name of some VnV::IReduction
// registered
//  with the CommunicationStore. <s> is the size of the vector on the "LOCAL"
//  processor. r is the process to reduce down to. NOTE: In this case we assume
//  all elements on the local process are part of some global vector. So,
//  calling this operation reduces across that vector. This is different to the
//  standard MPI_Reduce functions, which do an element wise reduction. For
//  ElementWise reduction use ElementWiseReductionAction instead.
class ReductionAction : public BaseAction {
 public:
  IReduction_ptr reducer;
  int size, root;
  ReductionAction(std::string red, int s = 1, int r = -1) : size(s), root(r) {
    reducer = ReductionStore::instance().getReducer(red);
  }
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    DataTypeCommunication d(comm);

    int rank = (root < 0) ? engine->getRoot(

                                )
                          : root;  // r<0 uses the engine root as root to save communication .

    IDataType_ptr result = d.ReduceVector(data, dtype, reducer, rank);
    std::vector<int> r(1);
    if (comm->Rank() == rank) {
      engine->PutGlobalArray(dtype, variableName, {result}, {1}, {1}, {0}, m);
    } else {
      engine->PutGlobalArray(dtype, variableName, {}, {1}, {0}, {0}, m);
    }
  }
  virtual int count(ICommunicator_ptr /* ptr */, int /* engineRoot */) const override { return size; };
};

// Apply a reduction across a global reducer. Here reducer is the name of some
// VnV::IReducer interface. root is the process to reduce the data too. root<0
// means the toolkit can decide the root. This is usually best as we can set the
// root process equal to the root process of the IOutputEngine.
template <typename T>
void IOutputEngine::Put_ReduceVector(std::string variableName, std::string reducer, std::vector<T>& data, int root,
                                     const MetaData& m) {
  Put_ReduceVector(variableName, reducer, data.size(), data.data(), root, m);
}

template <typename T>
void IOutputEngine::Put_ReduceVector(std::string variableName, std::string reducer, int size, T* data, int root,
                                     const MetaData& m) {
  ReductionAction action(reducer, size, root);
  Write(variableName, data, action, m);
}

template <typename T>
void IOutputEngine::Put_ReduceScalar(std::string variableName, std::string reducer, T& data, int root,
                                     const MetaData& m) {
  Put_ReduceVector(variableName, reducer, 1, &data, root, m);
}

// Reduce a global vector. Here reducer is the name of some VnV::IReduction
// registered
//  with the CommunicationStore. <s> is the size of the vector on the "LOCAL"
//  processor. r is the process to reduce down to. NOTE: In this case we do an
//  element wise reduction. ALL PROCESSORS MUST SET SIZE TO BE THE SAME OR THIS
//  WILL BREAK SPECTACULARLY

class ElementWiseReductionAction : public BaseAction {
 public:
  IReduction_ptr reducer;
  int size, root;
  ElementWiseReductionAction(std::string red, int s = 1, int r = -1) : size(s), root(r) {
    reducer = ReductionStore::instance().getReducer(red);
  }
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    // We end up with a vector of lenfth size on the root processor. We then
    // want to write a vector from the root processor with all those values. We
    // still run int through put global array anyway ?

    int rank = (root < 0) ? engine->getRoot() : root;  // r<0 uses the engine root as root to save communication .
    DataTypeCommunication d(comm);
    IDataType_vec result = d.ReduceMultiple(data, dtype, reducer, rank);
    std::vector<int> offs = {0};
    std::vector<int> lsize(1);
    lsize[0] = (comm->Rank() == rank) ? size : 0;
    if (comm->Rank() == rank) {
      engine->PutGlobalArray(dtype, variableName, result, {size}, {size}, {0}, m);
    } else {
      // No data if not the root.
      engine->PutGlobalArray(dtype, variableName, {}, {size}, {0}, {0}, m);
    }
  }
  virtual int count(ICommunicator_ptr /* ptr */, int /* engineRoot */) const override { return size; };
};

// Apply a reduction across a global reducer. Here reducer is the name of some
// VnV::IReducer interface. root is the process to reduce the data too. This
// reduces the vector element wise. So, in the end you get a vector the same
// size as data. This is similar to how MPI_Reduce works. Put_ReduceVector does
// a local reduction on the local vector and then performs an elementwise
// reduction on the result (a vector of length 1).
template <typename T>
void IOutputEngine::Put_ReduceVectorElementWise(std::string variableName, std::string reducer, int size, T* data,
                                                int root, const MetaData& m) {
  ElementWiseReductionAction action(reducer, size, root);
  Write(variableName, data, action, m);
}

template <typename T>
void IOutputEngine::Put_ReduceVectorElementWise(std::string variableName, std::string reducer, std::vector<T>& data,
                                                int root, const MetaData& m) {
  Put_ReduceVectorElementWise(variableName, reducer, data.size(), data.data(), root, m);
}

// Reduce the vector on the local processor and write one value per processor.
// If size is one, this is the same as Scalar Action.
class ProcessorWiseReductionAction : public ElementWiseReductionAction {
 public:
  ProcessorWiseReductionAction(std::string red, int s = 1, int r = -1) : ElementWiseReductionAction(red, s, r) {}
  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    DataTypeCommunication d(comm);
    IDataType_ptr result = d.ReduceLocalVec(data, reducer);
    engine->PutGlobalArray(dtype, variableName, {result}, {comm->Size()}, {1}, {comm->Rank()}, m);
  }
};

// This one reduces the vector processor wise. You end up with a vector of
// length = MPI_COMM_SIZE
//  where each element v_r is the result of the reduction applied to the local
//  vector on rank r. This one does a local reduction then calls PutGlobalArray
//  (rather than allReducing to the root and writing a local array.
template <typename T>
void IOutputEngine::Put_ReduceVectorRankWise(std::string variableName, std::string reducer, int size, T* data, int root,
                                             const MetaData& m) {
  ProcessorWiseReductionAction action(reducer, size, root);
  Write(variableName, data, action, m);
}

template <typename T>
void IOutputEngine::Put_ReduceVectorRankWise(std::string variableName, std::string reducer, std::vector<T>& data,
                                             int root, const MetaData& m) {
  Put_ReduceVectorRankWise(variableName, reducer, data.size(), data.data(), root, m);
}

class SingleProcessorReductionAction : public ProcessorWiseReductionAction {
 public:
  SingleProcessorReductionAction(std::string red, int s = 1, int r = -1) : ProcessorWiseReductionAction(red, s, r) {}

  virtual void write(ICommunicator_ptr comm, long long dtype, std::string variableName, IDataType_vec data,
                     IOutputEngine* engine, const MetaData& m) const override {
    int rank = (root < 0) ? engine->getRoot() : root;  // r<0 uses the engine root as root to save communication .
    DataTypeCommunication d(comm);
    IDataType_ptr result;
    if (comm->Rank() == rank) {
      result = d.ReduceLocalVec(data, reducer);
    }
    std::vector<int> offs(1, 0);
    std::vector<int> lsize(1);
    lsize[0] = (comm->Rank() == rank) ? 1 : 0;
    if (comm->Rank() == rank) {
      engine->PutGlobalArray(dtype, variableName, {result}, {1}, {1}, {0}, m);
    } else {
      engine->PutGlobalArray(dtype, variableName, {}, {1}, {0}, {0}, m);
    }
  }

  virtual int count(ICommunicator_ptr comm, int engineRoot) const override {
    int rank = (root < 0) ? engineRoot : root;  // r<0 uses the engine root as root to save communication .
    return (comm->Rank() == rank) ? size : 0;
  }
};

// This one reduces the vector on rank == root and writes the result as a scalar
// value. This is
//  a local reduction on a single core. Use this one if the data has already
//  been reduced to a single processor for some other reason.
template <typename T>
void IOutputEngine::Put_ReduceVectorRankOnly(std::string variableName, std::string reducer, int size, T* data, int root,
                                             const MetaData& m) {
  SingleProcessorReductionAction action(reducer, size, root);
  Write(variableName, data, action, m);
}

template <typename T>
void IOutputEngine::Put_ReduceVectorRankOnly(std::string variableName, std::string reducer, std::vector<T>& data,
                                             int root, const MetaData& m) {
  Put_ReduceVectorRankOnly(variableName, reducer, data.size(), data.data(), root, m);
}

class IInternalOutputEngine : public IOutputEngine {
 public:
  virtual void setFromJson(ICommunicator_ptr worldComm, json& configuration) = 0;

  virtual void sendInfoNode(ICommunicator_ptr worldComm, const json& fullJson, const json& prov, std::string workflowName, std::string workflowJob ) = 0;

  virtual void write_stdout_info(ICommunicator_ptr selfComm, const std::string& out, bool error) = 0;

  virtual void injectionPointStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string id,
                                             InjectionPointType type, std::string stageId, std::string filename,
                                             int line) = 0;

  virtual void injectionPointEndedCallBack(std::string id, InjectionPointType type, std::string stageId) = 0;

  virtual void actionStartedCallBack(ICommunicator_ptr /* comm */, std::string /* package */, std::string /* name */,
                                     std::string /* stage */){};
  virtual void actionEndedCallBack(std::string /* stage */){};

  virtual void workflowStartedCallback(ICommunicator_ptr /* comm */, std::string /* package */, std::string /* name */,
                                       const json& /* info */) {
    throw "Workflows not supported by this engine;";
  };
  virtual void workflowEndedCallback(ICommunicator_ptr /* comm */, std::string /* package */, std::string /* name */,
                                     const json& /* info */) {
    throw "Workflows not supported by this engine;";
  };

  virtual void workflowUpdatedCallback(ICommunicator_ptr /* comm */, std::string /* package */, std::string /* name */,
                                       const json& /* info */) {
    throw "Workflows not supported by this engine;";
  };

  virtual void testStartedCallBack(std::string packageName, std::string testName, bool internal, long uuid) = 0;
  virtual void testFinishedCallBack(bool result_) = 0;

  virtual void unitTestStartedCallBack(ICommunicator_ptr comm, std::string packageName, std::string unitTestName) = 0;

  virtual void unitTestFinishedCallBack(IUnitTest* tester) = 0;

  virtual void packageOptionsStartedCallBack(ICommunicator_ptr comm, std::string packageName) = 0;
  virtual void packageOptionsEndedCallBack(std::string packageName) = 0;

  virtual void initializationStartedCallBack(ICommunicator_ptr comm, std::string packageName) = 0;
  virtual void initializationEndedCallBack(std::string packageName) = 0;

  virtual void file(ICommunicator_ptr comm, std::string packageName, std::string name, bool inputFile, std::string reader, std::string infilename, std::string outfilename) = 0;

  virtual std::string print() = 0;

  virtual json getRunInfo() = 0;

  virtual void finalize(ICommunicator_ptr worldComm, long currentTime) = 0;

  virtual ~IInternalOutputEngine() = default;
};

/**
 * @brief The OutputEngineManager class
 */
class OutputEngineManager : public IInternalOutputEngine {
  std::string key;

 public:
  std::string getKey() { return key; };

  void set(ICommunicator_ptr world, json& configuration, std::string key);

  IOutputEngine* getOutputEngine();

  virtual void write_stdout(const std::string& out, bool error) {
      auto selfcomm = CommunicationStore::instance().selfComm();
      write_stdout_info(selfcomm, out, error);
  }

  virtual ~OutputEngineManager() = default;
};
typedef std::string (*engine_schema_ptr)();

typedef OutputEngineManager* (*engine_register_ptr)();
void registerEngine(std::string name, VnV::engine_register_ptr r, VnV::engine_schema_ptr s);

}  // namespace VnV

#define INJECTION_ENGINE(PNAME, name, schema)                                     \
  namespace VnV {                                                                 \
  namespace PNAME {                                                               \
  namespace Engines {                                                             \
  OutputEngineManager* declare_##name();                                          \
  std::string sch_##name() { return schema; }                                     \
  void register_##name() { registerEngine(#name, &declare_##name, &sch_##name); } \
  }                                                                               \
  }                                                                               \
  }                                                                               \
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
