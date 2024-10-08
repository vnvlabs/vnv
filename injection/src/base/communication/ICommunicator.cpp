﻿#include "base/communication/ICommunicator.h"

#include <iostream>

#include "shared/exceptions.h"
#include "base/stores/CommunicationStore.h"
#include "base/stores/DataTypeStore.h"
#include "base/stores/ReductionStore.h"
#include "interfaces/IOutputEngine.h"
#include "interfaces/IReduction.h"

VnV::IRequest::~IRequest() {
  if (buffer) {
    free(buffer);
  }
}

VnV::IDataType_vec VnV::IRequest::unpack() {
  if (ready && buffer && recv) {
    long long* buff = (long long*)buffer;
    long long dataType = buff[0];
    IDataType_vec vec;
    IDataType_ptr ptrA = DataTypeStore::instance().getDataType(dataType);
    long dataSize = ptrA->maxSize();

    VnV::IDataType_vec results;
    char* cbuff = (char*)buffer;
    for (int i = 0; i < count; i++) {
      ptrA = DataTypeStore::instance().getDataType(dataType);
      long long* buff = (long long*)&(cbuff[i * dataSize]);
      ptrA->unpack(&(buff[1]));
      results.push_back(ptrA);
    }

    free(buffer);
    buffer = nullptr;
    return results;
  } else {
    throw INJECTION_EXCEPTION(
        "Attempted to unpack Request that was not marked ready or already "
        "unpacked or not a recv (ready : %d, buffer: %d, recv: %d",
        ready, buffer, recv);
  }
}

void VnV::OpTypeEncodedReduction(void* invec, void* outvec, int* len) {
  long long* buff = (long long*)invec;
  long long reducerKey = buff[0];
  long long dataKey = buff[1];

  long dataSize = DataTypeStore::instance().getDataType(dataKey)->maxSize() + 2 * sizeof(long long);
  IReduction_ptr reducer = ReductionStore::instance().getReducer(reducerKey);

  // Call the reduction across all processors.
  char* cinvec = (char*)invec;
  char* coutvec = (char*)outvec;

  for (int i = 0; i < *len; i++) {
    buff = (long long*)&(cinvec[i * dataSize]);
    auto in = DataTypeStore::instance().getDataType(dataKey);
    in->unpack(&(buff[2]));

    buff = (long long*)&(coutvec[i * dataSize]);
    auto out = DataTypeStore::instance().getDataType(dataKey);
    out->unpack(&(buff[2]));

    out = reducer->reduce(in, out);
    out->pack(&(buff[2]));

  }

}
VnV::IStatus::~IStatus() {}



