#include "interfaces/ICommunicator.h"
#include "base/CommunicationStore.h"
#include "base/exceptions.h"

VnV::Communication::IRequest::~IRequest() {
     if (buffer) {
               free(buffer);
          }
}

VnV::Communication::IDataType_vec VnV::Communication::IRecvRequest::unpack() {
     if (ready && buffer) {
               long long* buff = (long long*) buffer;
               long long dataType = buff[0];
               IDataType_vec vec;
               IDataType_ptr ptrA = CommunicationStore::instance().getDataType(dataType);
               long dataSize = ptrA->maxSize();

               VnV::Communication::IDataType_vec results;
               char * cbuff = (char*) buffer;
               for (int i = 0; i < count; i++ ) {
                         ptrA = CommunicationStore::instance().getDataType(dataType);
                         long long *buff = (long long*) &(cbuff[i*dataSize]);
                         ptrA->unpack(&(buff[1]));
                         results.push_back(ptrA);
               }

               free(buffer);
               buffer = nullptr;
               return results;
          } else {
               throw VnV::VnVExceptionBase("Attempted to unpack RecvRequest that was not marked ready or already unpacked.");
          }
}

void VnV::Communication::OpTypeEncodedReduction(void *invec, void *outvec, int *len) {

     long long * buff = (long long *) invec;
     long long reducerKey = buff[0];
     long long dataKey = buff[1];
     long dataSize = CommunicationStore::instance().getDataType(dataKey)->maxSize() + 2*sizeof(long long);
     IReduction_ptr reducer = CommunicationStore::instance().getReducer(reducerKey);

     //Call the reduction across all processors.
     char* cinvec = (char*) invec;
     char* coutvec = (char*) outvec;

     for (int i = 0; i < *len; i++ ) {
               long long * buff = (long long *) &(cinvec[i*dataSize]);
               auto in = CommunicationStore::instance().getDataType(dataKey);
               in->unpack(&(buff[2]));

               buff = (long long *) &(coutvec[i*dataSize]);
               auto out = CommunicationStore::instance().getDataType(dataKey);
               out->unpack(&(buff[2]));
               reducer->reduce(in,out)->pack(&(buff[2]));
          }
}

long long VnV::Communication::IReduction::getKey() {
     return key;
}

void VnV::Communication::IReduction::setKey(long long key) {
     this->key = key;
}

void VnV::Communication::IDataType::setKey(long long key) {
     this->key = key;
}

long long VnV::Communication::IDataType::getKey() {
     return key;
}

VnV::Communication::IDataType::~IDataType() {

}
