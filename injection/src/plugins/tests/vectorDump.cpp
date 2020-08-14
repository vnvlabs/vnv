

#include "VnV.h"


/**
 * Test Writing a Vector to File
 * =============================
 *
 * The global size of the vector was :vnv:`len(Data.result)` `
 *
 * .. vnv-chart::
 *    :labels: Data.result.Children.Name
 *    :ydata: Data.result.Children.Value
 *
 *    {
 *       "type" : "line",
 *       "data" : {
 *          "labels" : $$labels$$,
 *          "datasets" : [{
 *             "label": "Random information",
 *             "backgroundColor": "rgb(255, 99, 132)",
 *             "borderColor": "rgb(255, 99, 132)",
 *             "data": $$ydata$$
 *           }]
 *       }
 *    }
 *
**/
INJECTION_TEST(VNVPACKAGENAME, vectorDump) {
  double val = comm->Rank();

  // Vector Action dumps one value from every processor into a global array
  engine->Write(comm, "result", &val, VnV::VectorAction());

  return SUCCESS;
}
