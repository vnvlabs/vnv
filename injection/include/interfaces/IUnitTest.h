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

#endif // IUNITTESTER_H
