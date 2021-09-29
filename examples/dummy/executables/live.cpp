
#include <mpi.h>
#include <time.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <typeinfo>
#include <vector>
#include <thread>
#include <chrono>

#include "VnV.h"

/**
 * This example demonstrates the "live" capabilities of the toolkit.
 * 
 * The example contains a single injection point called  
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

#define SPNAME Live

/**
 * 
 * A Little Test that calls fetch on iteration 40. 
 * The Fetch statment requests input from the user in 
 * the form of a json object that is validated against the
 * provided schema. 
 *
 * The timeout waits for 500 seconds (parameter 2 of the Fetch function. ).
 * Fetch returns a boolean indicating if the fetch was successful. 
 * 
 * x is a parameter because the iterface doesn;t allow a test with no parameters (todo)
 * 
 * YOU RESPONDED WITH THE VALUE: :vnv:`response[0]`
 * 
 */
INJECTION_TEST(Live, interupt, double x) {
  (*runner)++;  
  
  if (*runner == 40) {
    nlohmann::json schema = R"({"type":"object"})"_json;
    nlohmann::json response = R"({"type":"object"})"_json;
    if ( engine->Fetch("Hello From Me", schema,500,response) ) {
      
        engine->Put("response", response.dump(4));
    } else {
       engine->Put("response", "N/A: (Fetch timed out) ");
    
    }
  }
  return SUCCESS;
}


/**
 * A simple Application highlighting "live" report generation 
 * and two-way communication between the gui and the application. 
 * 
 * 
 */
INJECTION_EXECUTABLE(SPNAME)


/**
 * @brief The main function
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char** argv) {
  
  if (argc == 1 ) {
      std::cout << "Usage: ./live.a <int:time_in_seconds>" << std::endl;
      std::cout << "\n\nThis application with run for about \"time in seconds\". It will ouput some data to " 
                << "the ./vv-output-live directory.\nOnce the application is running "  
                << "open the gui (cd <build>/report-generation && run.py then go localhost:5000) and "
                << "load the output file (this_directory/vv-output-live). The gui should give you a live view"
                << "of the application, including a graph showing the cputime updating in real time. After about"
                << " 40 seconds a request will be sent to the \"Sample Live Loop\" injection point (click it in the graph)"
                << " You should enter some valid json in the form and click respond. This demonstrates the computational steering"
                << "support in the toolkit. ( the animation sucks -- I am working on it ) ";
      
      return 0;
  }


  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /**
   * Simple Executable
   * =================
   *
   * This example illustrates the "live" capabilities on the vnv toolkit. 
   * 
   * 
   * Here is a math directive -- which is also cool! 
   *  
   * .. math::
   * 
   *   f(x,y) = x^{2} + y^{2} - \frac{x}{y}
   * 
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv,"./vv-input-live.json");

  double f = 0;
  double x = 0;
  
  /**
   * The Test injection loop. This is an injection point loop wrapped 
   * around a while loop containing a one second sleep. The Injection 
   * point loop contains an injection iteration within this loop. 
   * 
   * The parameter x is a counter counting the number of iterations that have passed. 
   */
  INJECTION_LOOP_BEGIN("Live", VWORLD, "Sample Live Loop", x);

  while (x < std::atoi(argv[1])) {
    INJECTION_LOOP_ITER("Live","Sample Live Loop", "iter");
    std::this_thread::sleep_for (std::chrono::seconds(1));
    x++;
    std::cout << "Countdown: " << std::atoi(argv[1]) - x << std::endl;
  } 

  INJECTION_LOOP_END("Live", "Sample Live Loop");
  
  
  INJECTION_LOOP_BEGIN("Live", VWORLD, "Sample Live Loop1", x);

  x = 0;
  while (x < std::atoi(argv[1])) {
    INJECTION_LOOP_ITER("Live","Sample Live Loop1", "iter");
    s/td::this_thread::sleep_for (std::chrono::seconds(1));
    x++;
    std::cout << "Countdown1: " << std::atoi(argv[1]) - x << std::endl;
  } 

  INJECTION_LOOP_END("Live", "Sample Live Loop1");

  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
