

#include "plugins/engines/adios_patch/ifstream.h"

namespace adios2 {

template <class T> std::vector<T> StreamPatch::Read(const std::string& name, const size_t blockID) {
  Variable<T> variable = m_IO.InquireVariable<T>(name);
  if (!variable) {
    return std::vector<T>();
  }
  SetBlockSelectionCommon(variable, blockID);
  return GetCommon(variable);
}

template <class T> void StreamPatch::SetBlockSelectionCommon(Variable<T>& variable, const size_t blockID) {
  if (variable.ShapeID() == ShapeID::LocalArray) {
    variable.SetBlockSelection(blockID);
  } else {
    if (blockID != 0) {
      throw std::invalid_argument("ERROR: in variable " + variable.Name() +
                                  " only set blockID > 0 for variables "
                                  "with ShapeID::LocalArray, in call to read\n");
    }
  }
}

void StreamPatch::CheckOpen() {
  if (!m_Engine) {
    m_Engine = m_IO.Open(m_Name, adios2::Mode::Read);
  }
}

template <class T> 
std::vector<T> StreamPatch::GetCommon(Variable<T>& variable) {
  try {
    std::vector<T> values(variable.SelectionSize());
    CheckOpen();
    m_Engine.Get(variable, values.data(), adios2::Mode::Sync);
    return values;
  } catch (std::exception& e) {
    std::throw_with_nested(std::runtime_error("ERROR: couldn't read variable " + variable.Name() + "\n" + e.what()));
  }
}

StreamPatch::StreamPatch(std::string name) : m_Name(name), 
 m_ADIOS(), m_IO(m_ADIOS.DeclareIO(name)) {
     CheckOpen();
 }

#if ADIOS2_USE_MPI
StreamPatch::StreamPatch(const std::string& name, MPI_Comm comm) :
 m_ADIOS(comm), m_IO(m_ADIOS.DeclareIO(name)) {
     CheckOpen();
 }
 
 
#endif

size_t StreamPatch::CurrentStep() const noexcept {
  if (m_FirstStep) {
    return 0;
  }

  if (m_Engine) {
    throw std::invalid_argument("ERROR: stream with name " + m_Name +
                                "is invalid or closed, in call "
                                "to CurrentStep");
  }

  return m_Engine.CurrentStep();
}

bool StreamPatch::GetStep(double timeoutSeconds) {
  if (!m_FirstStep) {
    if (m_StepStatus) {
      m_Engine.EndStep();
    }
  } else {
    m_FirstStep = false;
  }


  if (m_Engine.BeginStep(StepMode::Read, timeoutSeconds) != StepStatus::OK) {
    m_StepStatus = false;
    return false;

  }



  m_StepStatus = true;
  return true;
}

void StreamPatch::Close() {
  if (m_Engine) {
    m_Engine.Close();
    m_StepStatus = false;
  }
}

void StreamPatch::EndStep() {
  if (m_StepStatus) {
    m_Engine.EndStep();
    m_StepStatus = false;
  } else {
    throw std::invalid_argument("ERROR: stream " + m_Name +
                                " calling end step function twice (check "
                                "if a write function calls it) or "
                                "invalid stream\n");
  }
}

std::size_t StreamPatch::Steps() {
  const size_t steps = (m_Engine) ? m_Engine.Steps() : 0;
  return steps;
}

#define declare_template_instantiation(T)                                      \
                                                                               \
    template std::vector<T> StreamPatch::Read<T>(const std::string &,               \
                                            const size_t);                     \
                                                                               
ADIOS2_FOREACH_STDTYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation



#if ADIOS2_USE_MPI
MPI_Comm dupComm(MPI_Comm comm) {
  MPI_Comm dup;
  MPI_Comm_dup(comm, &dup);
  return dup;
}
#endif

template <class T> std::vector<T> ifstream::read(const std::string& name, const size_t blockID) {
  using IOType = typename TypeInfo<T>::IOType;
  auto vec = m_Stream->Read<IOType>(name, blockID);
  return reinterpret_cast<std::vector<T>&>(vec);
}

ifstream::ifstream(const std::string& name) : m_Stream(std::make_shared<StreamPatch>(name)) {}
void ifstream::open(const std::string& name) {
    CheckOpen(name);
    m_Stream = std::make_shared<StreamPatch>(name);
}

#if ADIOS2_USE_MPI
ifstream::ifstream(const std::string& name, MPI_Comm comm) : m_Stream(std::make_shared<StreamPatch>(name, dupComm(comm))) {}

void ifstream::open(const std::string& name, MPI_Comm comm) {
    CheckOpen(name);
    m_Stream = std::make_shared<StreamPatch>(name, dupComm(comm));
}

#endif

ifstream::operator bool() const noexcept {
  if (!m_Stream) {
    return false;
  }
  return true;
}

void ifstream::CheckOpen(const std::string &name) const
{
    if (m_Stream)
    {
        throw std::invalid_argument("ERROR: adios2::fstream with name " + name +
                                    " is already opened, in call to open");
    }
}

void ifstream::end_step() { m_Stream->EndStep(); }

void ifstream::close() {
  m_Stream->Close();
  m_Stream.reset();
}

bool getstep(adios2::ifstream& stream, adios2::ifstep& step, double timeoutSeconds) {
  step = stream;
  return step.m_Stream->GetStep(timeoutSeconds);
}

size_t ifstream::current_step() const noexcept { return m_Stream->CurrentStep(); }

size_t ifstream::steps() const { return m_Stream->Steps(); }

// PRIVATE
void fstream::CheckOpen(const std::string& name) const {
  if (m_Stream) {
    throw std::invalid_argument("ERROR: adios2::fstream with name " + name + " is already opened, in call to open");
  }
}

#define declare_template_instantiation(T) template std::vector<T> ifstream::read<T>(const std::string&, const size_t);

ADIOS2_FOREACH_TYPE_1ARG(declare_template_instantiation)
#undef declare_template_instantiation

}  // end namespace adios2
