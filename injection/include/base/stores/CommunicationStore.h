#ifndef COMMUNICATIONSTORE_H
#define COMMUNICATIONSTORE_H

#include "common-interfaces/all.h"
#include "base/stores/BaseStore.h"
#include "base/communication/ICommunicator.h"

using VnV::comm_register_ptr;
using VnV::ICommunicator;
using VnV::ICommunicator_ptr;

namespace VnV
{

  class CommunicationStore : public BaseStore
  {
    friend class Runtime;

  private:
    // Using long long here as the key because we want to be able to pass these
    // keys across processors in buffers. Long long avoids sending the char
    // arrays.
    ICommunicator_ptr root;

  public:
    CommunicationStore();

    VnV_Comm toVnVComm(ICommunicator_ptr ptr);
    VnV_Comm toVnVComm(ICommunicator *ptr);
    VnV_Comm world();
    VnV_Comm self();

#ifndef WITHOUT_MPI
    VnV_Comm custom(MPI_Comm comm);
#endif

    ICommunicator_ptr getCommunicator(VnV_Comm comm);
    ICommunicator_ptr worldComm();
    ICommunicator_ptr selfComm();

    void Finalize();

    static CommunicationStore &instance();
  };

} // namespace VnV

#endif // COMMUNICATIONSTORE_H
