#ifndef IUNITTESTER_H
#define IUNITTESTER_H
#include <map>
#include "interfaces/IOutputEngine.h"

namespace VnV {

class IUnitTester {
 public:
  /**
   * @brief IUnitTester
   */
  IUnitTester();

  /**
   * @brief ~IUnitTester
   */
  virtual ~IUnitTester();

  /**
   * @brief run
   */
  virtual std::map<std::string, bool> run(IOutputEngine *engine) = 0;


};

typedef IUnitTester* tester_ptr();
void registerUnitTester(std::string name, VnV::tester_ptr ptr);

}

#endif // IUNITTESTER_H
