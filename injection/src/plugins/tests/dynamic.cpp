#include <dlfcn.h>

#include <iostream>
#include <string>

#include "VnV.h"
#include "base/DistUtils.h"

namespace {

typedef VnV::ITest* dynamic_test_maker_ptr(VnV::TestConfig config);

VnV::ITest* loadPlugin(std::string libraryPath, std::string symbolName,
                       VnV::TestConfig config) {
  try {
    void* dllib = VnV::DistUtils::loadLibrary(libraryPath);

    if (dllib != nullptr) {
      void* callback = dlsym(dllib, symbolName.c_str());
      if (callback != nullptr) {
        return ((dynamic_test_maker_ptr*)callback)(config);
      }
      throw VnV::VnVExceptionBase("Library Registration Symbol not found");
    }
    throw VnV::VnVExceptionBase("Library not found");
  } catch (...) {
    throw VnV::VnVExceptionBase("Library not found");
  }
}

void talk(std::string& filename, std::string& symbol) {
  std::cout << "You have triggered the dynamic library loader.\n"
            << "Please enter a filename to a dynamic library compaitable "
               "kernal (empty to skip)"
            << std::endl;

  std::getline(std::cin, filename);
  if (!filename.empty()) {
    std::cout << "Thanks, now enter the name of the symbol inside the library";
    std::getline(std::cin, symbol);
    if (!symbol.empty()) {
      std::cout << "Thanks -- loading the library and quiting. ";
    } else {
      std::cout << "Ok, we will continue without testing.";
    }
    std::cout << "Thanks -- loading the library -- see you next time. ";
  } else {
    std::cout << "No Prob -- see you next time. ";
  }
}

}  // namespace

/**
 * This is a proof of concept test to prove we can dynamically load
 * tests at runtime. When triggered, this test asks the user to provide
 * a filename and a symbol name for a function that creates a new test.
 *
 */
INJECTION_TEST(VNVPACKAGENAME, dynamicTestLoader) {
  TestStatus s = FAILURE;

  // Keep asking the user to load a library until there kernal
  // returns success.
  while (s != SUCCESS) {
    // Get the filename a nd symbol to load
    std::string filename;
    std::string symbol;
    if (comm->Rank() == 0) {
      talk(filename, symbol);
    }

    // Broadcast the filename and symbol to everyone.
    int d[2] = {filename.length(), symbol.length()};
    comm->BroadCast(d, 2, sizeof(int), 0);

    // Everyone loads the shared library if we recieved a valid filename and
    // symbol.
    if (d[0] > 0 && d[1] > 0) {
      filename = filename + symbol;
      std::vector<char> cdata(filename.begin(), filename.end());
      comm->BroadCast(cdata.data(), cdata.size(), sizeof(char), 0);

      std::string fname(cdata.begin(), cdata.begin() + d[0]);
      std::string sname(cdata.begin() + d[0], cdata.end());

      // Create the object
      std::shared_ptr<ITest> test(loadPlugin(fname, symbol, m_config));

      // Run the test (shared_ptr destroys it)
      s = test->runTest(comm, engine, type, stageId);
    }
  }
  return SUCCESS;
}

// Notes:

/**
 * This is a proof of concept for computational steering using the vnv
 * framework.
 *
 * The idea is that if we can ask the user to provide a shared library and load
 * it during a test, then we can "steer" the application by allowing the user to
 * modify the parameters inside that shared library.
 *
 * In this example I used the existing ITest infrastructure. That gives the
 * shared librray access to the variables -- so they definetly can steer using
 * this.
 *
 * One thing that we do need to figure out is what comment to use for the test
 * template.
 *
 * Using the ITest interface is somewhat unintuitive, might be benefiical to
 * subclass it and add a new IKernal/Steering/Debug class.
 *
 */
