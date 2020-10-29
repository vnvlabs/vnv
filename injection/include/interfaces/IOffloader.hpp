#ifndef IOFFLOADER_HPP
#define IOFFLOADER_HPP

#include <string>
#include "interfaces/ITest.h"

namespace VnV {

class IOffloader {
public:
     virtual std::string offload(ITest* test) = 0;
     virtual void config(json config) = 0;
     virtual ~IOffloader() {}
};

typedef IOffloader* offloader_register_ptr();

}


#endif // IOFFLOADER_HPP
