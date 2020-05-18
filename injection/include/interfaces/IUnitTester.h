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

#define INJECTION_UNITTEST_R(name, Runner) \
namespace VnV{ \
namespace PACKAGENAME {\
namespace UnitTests {\
class name : public IUnitTester { \
public:\
    std::shared_ptr<Runner> runner; \
    name() : IUnitTester() { \
        runner.reset(new Runner()); \
    }\
    std::map<std::string,bool> run(IOutputEngine *engine) override; \
\
}; \
IUnitTester* declare_##name() { return new name(); } \
void register_##name() { \
    registerUnitTester(#name, declare_##name); \
} \
} \
} \
} \
std::map<std::string,bool> VnV::PACKAGENAME::UnitTests::name::run(IOutputEngine *engine)

#define INJECTION_UNITTEST(name,...) INJECTION_UNITTEST_R(name,int)

#define DECLAREUNITTEST(name) \
  namespace VnV { namespace PACKAGENAME { namespace UnitTests { void register_##name(); } } }

#define REGISTERUNITTEST(name) \
  VnV::PACKAGENAME::UnitTests::register_##name();


#endif // IUNITTESTER_H
