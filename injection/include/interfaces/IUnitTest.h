#ifndef IUNITTESTER_H
#define IUNITTESTER_H
#include <vector>
#include <stdexcept>

namespace VnV {

class IUnitTest {
 protected:
    std::vector<std::tuple<std::string, std::string, bool>> results;
    bool continue_on_fail = true;

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
  std::vector<std::tuple<std::string, std::string, bool>> getResults() { return results; }

  /**
   * @brief stop/continue on test failure
   */
  void setContinueOnFail(bool flag) { continue_on_fail = flag; }

};

typedef IUnitTest* tester_ptr();
void registerUnitTester(std::string name, VnV::tester_ptr ptr);

}

#define TEST_ASSERT_EQUALS(name, expected, got)                         \
    {                                                                   \
        if (!((got) == (expected)))                                     \
        {                                                               \
            std::stringstream tmpstream;                                \
            tmpstream << "Got " << (got) << ", expected " << (expected);\
            results.emplace_back((name), tmpstream.str(), false);       \
            if (!continue_on_fail) throw std::runtime_error((name));    \
        } else {                                                        \
            results.emplace_back((name), "", true);                     \
        }                                                               \
    }

template <typename Runner>
class UnitTester_T : public IUnitTest {
public:
  std::shared_ptr<Runner> runner;
  UnitTester_T() {
     runner.reset(new Runner());
  }
};


}

#define INJECTION_UNITTEST_R(name, Runner) \
namespace VnV{ \
namespace PACKAGENAME {\
namespace UnitTests {\
class name : public VnV::UnitTester_T<VnV_Arg_Type(Runner)> { \
public:\
    name() : VnV::UnitTester_T<VnV_Arg_Type(Runner)>() {}\
    virtual std::map<std::string,bool> run(IOutputEngine *engine) override; \
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
