
#ifndef ADIOS2_PATCH_H_
#define ADIOS2_PATCH_H_

#include <memory>  //std::shared_ptr

#if ADIOS2_USE_MPI
#  include <mpi.h>
#endif

#include "adios2.h"

namespace adios2 {

class ifstream;
typedef ifstream ifstep;

class StreamPatch {
 public:
  StreamPatch(std::string name);
#if ADIOS2_USE_MPI
  StreamPatch(const std::string& name, MPI_Comm comm);
#endif
  size_t CurrentStep() const noexcept;
  bool GetStep(double timeoutSeconds = -1);
  void Close();
  void EndStep();
  std::size_t Steps();
  void CheckOpen();
  template <class T> std::vector<T> Read(const std::string& name, const size_t blockID);

 private:
  template <class T> std::vector<T> GetCommon(Variable<T>& variable);
  template <class T> void SetBlockSelectionCommon(Variable<T>& variable, const size_t blockID);

  ADIOS m_ADIOS;
  Engine m_Engine;
  IO m_IO;
  bool m_StepStatus = false;
  bool m_FirstStep = true;
  std::string m_Name;
};

// Explicit declaration of the public template methods
#define declare_template_instantiation(T) \
  extern template std::vector<T> StreamPatch::Read<T>(const std::string&, const size_t);

ADIOS2_FOREACH_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

class ifstream {
 public:
#if ADIOS2_USE_MPI
  ifstream(const std::string& name, MPI_Comm comm);
  void open(const std::string& name, MPI_Comm comm);
#endif

  ifstream(const std::string& name);
  ifstream(){};
  void open(const std::string& name);

  ~ifstream() = default;

  explicit operator bool() const noexcept;

  template <class T> std::vector<T> read(const std::string& name, const size_t blockID = 0);

  void end_step();

  void close();

  friend bool getstep(adios2::ifstream& stream, adios2::ifstep& step, double timeout);

  size_t current_step() const noexcept;

  size_t steps() const;

 protected:
  std::shared_ptr<StreamPatch> m_Stream;

 private:
  ifstream(ifstream& stream) = default;

  void CheckOpen(const std::string& name) const;
};

#define declare_template_instantiation(T) \
                                          \
  extern template std::vector<T> ifstream::read<T>(const std::string&, const size_t);

ADIOS2_FOREACH_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

}  // end namespace adios2

#endif /* ADIOS2_BINDINGS_CXX11_CXX11_FSTREAM_ADIOS2FSTREAM_H_ */
