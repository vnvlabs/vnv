#ifndef IUNITTESTER_H
#define IUNITTESTER_H
#include <memory>
#include <stdexcept>
#include <vector>

#include "interfaces/ICommunicator.h"

namespace VnV {

typedef std::vector<std::tuple<std::string, std::string, bool>> UnitTestResults;

class IUnitTest {
 protected:
  UnitTestResults results;
  bool continue_on_fail = true;
  ICommunicator_ptr comm;

 public:
  /**
   * @brief IUnitTester
   */
  IUnitTest();

  /**
   * @brief ~IUnitTester
   */
  virtual ~IUnitTest();

  /**
   * @brief run the tests
   */
  virtual void run() = 0;

  /**
   * @brief get the results from the tests
   */
  UnitTestResults getResults() { return results; }

  void setComm(ICommunicator_ptr ptr) { comm = ptr; }
  ICommunicator_ptr getComm() { return comm; }

  /**
   * @brief stop/continue on test failure
   */
  void setContinueOnFail(bool flag) { continue_on_fail = flag; }
};

typedef IUnitTest* (*tester_ptr)();
void registerUnitTester(std::string packageName, std::string name,
                        VnV::tester_ptr ptr, int cores);

template <typename Runner> class UnitTester_T : public IUnitTest {
 public:
  std::shared_ptr<Runner> runner;
  UnitTester_T() { runner.reset(new Runner()); }
};

}  // namespace VnV

#define TEST_ASSERT_EQUALS(name, expected, got)                    \
  {                                                                \
    if (!((got) == (expected))) {                                  \
      std::stringstream tmpstream;                                 \
      tmpstream << "Got " << (got) << ", expected " << (expected); \
      results.emplace_back((name), tmpstream.str(), false);        \
      if (!continue_on_fail) throw std::runtime_error((name));     \
    } else {                                                       \
      results.emplace_back((name), "", true);                      \
    }                                                              \
  }

#define TEST_ASSERT_NOT_EQUALS(name, expected, got)                \
  {                                                                \
    if (((got) == (expected))) {                                   \
      std::stringstream tmpstream;                                 \
      tmpstream << "Got " << (got) << ", expected " << (expected); \
      results.emplace_back((name), tmpstream.str(), false);        \
      if (!continue_on_fail) throw std::runtime_error((name));     \
    } else {                                                       \
      results.emplace_back((name), "", true);                      \
    }                                                              \
  }

#define INJECTION_UNITTEST_R(PNAME, name, Runner, cores)               \
  namespace VnV {                                                      \
  namespace PNAME {                                                    \
  namespace UnitTests {                                                \
  class name : public VnV::UnitTester_T<VnV_Arg_Type(Runner)> {        \
   public:                                                             \
    name() : VnV::UnitTester_T<VnV_Arg_Type(Runner)>() {}              \
    virtual void run() override;                                       \
  };                                                                   \
  IUnitTest* declare_##name() { return new name(); }                   \
  void register_##name() {                                             \
    registerUnitTester(VNV_STR(PNAME), #name, &declare_##name, cores); \
  }                                                                    \
  }                                                                    \
  }                                                                    \
  }                                                                    \
  void VnV::PNAME::UnitTests::name::run()

#define INJECTION_UNITTEST(PNAME, name, cores) \
  INJECTION_UNITTEST_R(PNAME, name, int, cores)

#define INJECTION_UNITTEST_RAW(PNAME, name, cls, cores)        \
  namespace VnV {                                              \
  namespace PNAME {                                            \
  namespace UnitTests {                                        \
  IUnitTest* declare_##name() { return new cls(); }            \
  void register_##name() {                                     \
    registerUnitTester(#PNAME, #name, &declare_##name, cores); \
  }                                                            \
  }                                                            \
  }                                                            \
  }

#define DECLAREUNITTEST(PNAME, name) \
  namespace VnV {                    \
  namespace PNAME {                  \
  namespace UnitTests {              \
  void register_##name();            \
  }                                  \
  }                                  \
  }

#define REGISTERUNITTEST(PNAME, name) VnV::PNAME::UnitTests::register_##name();

#endif  // IUNITTESTER_H
