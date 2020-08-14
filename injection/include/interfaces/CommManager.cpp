#include <vector>
#include "interfaces/ICommunicator.h"

namespace VnV {

// Class designed to manage communication maps for use in the end.

// We are looking for a data structure that describes all the comms in
// used in the executable. The data structure should be as small as possible
// while showing all the

typedef std::vector<uint_32> CompressedCommData;

class CommManager {

   std::map<int, CommpressedCommData> data;
public:

     CommManager() {

     }

     void addComm(Communication::ICommunicator_ptr comm) {

     }


}


}
