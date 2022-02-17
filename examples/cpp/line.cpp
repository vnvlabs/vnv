
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <mpi.h>
#include <map>
#include <utility>

#include "VnV.h"

using namespace VnV;


 /**
   * @title Line Plot Example. (as a third party test)
   * 
   * In this example we plot a simple line plot using the Plotly interface. A number of different
   * line chart options are presented. 
   * 
   * The vnv plotly interface is a thin RST wrapper around the plotly javascript/json api. We support 
   * a large majority of the options available within that api. Function type parameters are not supported
   * at this time.
   * 
   * The Plotly Javascript API uses **traces** to allow users to define multiple series within a single chart. Each
   * **trace** is a json object containing the data and configuration parameters that should be applied to that 
   * particular data series.
   * 
   *  In the VnV API you define **traces** using the *:trace.<name>: <chart-type>* option. For example, 
   * in the following plot, the user has defined a trace called main that will be rendered as a scatter chart. You can then set options 
   * within that trace using options of the form *:<name>.<option>: <value>*. In second and third options of the chart below, we use the 
   * *main.x* and *main.y* options to set the **x** and **y** data for the scatter plot. Layout options are set using the layout.* set of options.
   * Layout options represent global options applied to the chart as a whole, independent of the options set for each individual trace.   
   * 
   * 
   * Users can use the VnV specific <vnv-jmes-path> command to inject into the traces and layout configuration. The VnV report generation
   * software uses regular expressions to replace all commands matching this pattern prior to rendering the chart. JMES Path is feature rich
   * JSON query language designed to allow users to quickly and efficiently query json objects. VnV uses a (very slightly) modified implementation
   * of the python jmespath reference implementation to enable jmespath queries to be used to query the VnV simulation data object. 
   * 
   * In a VnV test (such as this one), all jmes path queries are applied against a json-object-like object containing all data exported using 
   * the engine->Put() calls during test execution. For instance, lets say a test makes the following set of calls within a VnV test.
   * 
   * .. code:: cpp
   * 
   *    engine->Put("x", 100)
   *    engine->Put("y", 200)
   *    engine->Put("y", 300)
   *    engine->Put_Vector("z", {1,2,3,4})
   *    engine->Put("title", "This is the title")
   * 
   * Then, the VnV map-like object looks something like this:
   * 
   * .. code:: json
   * 
   *    {
   *        "x" : [100],
   *        "y" : [200,300],
   *        "z" : [[1,2,3,4]],
   *        "title" : ["This is the title"]
   *    }
   * 
   * The most important thing to notice here is that the map-like object maps parameter names to arrays. For example, if this were a python dict,
   * a command like object["x"[0] would be required to fetch the value written for the variable **x**. This may seem a little counterintuitive, however,
   * we chose to do this because it allows for better data collection across tests. For example, in the example above, the user has made to calls to 
   * **engine->Put("y",...). In the map like object, this is represented as a vector of length=2. That is to say, within the context of the test, VnV 
   * collects values with the same name into a vector. This makes it infinetly easier to collect and plot data within injection point loops. (recall from 
   * the injection point example -- tests can be called any number of times. ) 
   *   
   * For a full  tutorial on JMES Path and VnV please see the VnV JMES Path tutorial `todo <#>`_. VnV supports the entire JMESPath
   * query language, including features like vector slicing. 
   * 
   * 
   * Most of the examples below come from the Plotly javascript `simple line chart tutorial <https://plotly.com/javascript/line-charts/>`_ 
   * 
   * .. vnv-plotly::
   *    :trace.main: scatter
   *    :main.x: {{as_json(x[0])}}
   *    :main.y: {{as_json(y[0])}}
   *    :layout.title.text: The Time Was {{time[0]}} 
   * 
   * You can add two data series to the same chart by adding another trace
   * 
   * .. vnv-plotly::
   *    :trace.main: scatter
   *    :trace.second: scatter
   *    :main.x: {{as_json(x[0])}}
   *    :main.y: {{as_json(y[0])}}
   *    :second.x: {{as_json(x[0])}}
   *    :second.y: {{as_json(z[0])}}
   *    :layout.title.text: The time was {{time[0]}}
   * 
   * You can also remove the lines on a series and/or add a custom name to the series. Nested parameters
   * can also be set. Basically, you can add as much or as little information to the plots as you like. 
   * 
   * 
   * .. vnv-plotly::
   *    :trace.main: scatter
   *    :trace.second: scatter
   *    :main.x: {{as_json(x[0])}}
   *    :main.y: {{as_json(y[0])}}
   *    :second.x: {{as_json(x[0])}}
   *    :second.y: {{as_json(z[0])}}
   *    :second.mode: markers
   *    :second.name: second series
   *    :second.marker.size: 10
   *    :layout.title.text: The time was {{time[0]}}
   *
   * 
   *  
   **/
INJECTION_TEST(LineExample, line) {
  
  //Tests have direct access to the variables passed by the injection point. The fastest way to access variables
  //in to use the GetRef_NoCheck macro. This macro takes two parameters, the variable name, and the class of the variable. 
  //VnV passes injection point parameters around using void* pointers. Thus, to access the injection point parameters, the 
  //user must know the type of the parameter. 

  // GetRef_NoCheck casts from void* through to <type>* before returning a <type>& reference. The NoCheck option turns off 
  // implicit VnV type checking (which is still a little buggy, exspecially when templates are involved.) Once the bugs are ironed
  // out, we will depreciate the GetRef_NoCheck macro. 


  auto t = GetRef_NoCheck("t", double);

  auto x = GetRef_NoCheck("x", std::vector<double>);
  auto y = GetRef_NoCheck("y", std::vector<double>);
  auto z = GetRef_NoCheck("z", std::vector<double>);
  
  // The engine object is used to store data. In this case, the user has stored the value "t" with the name "time". Statements 
  // of the type engine->Put() are considered to be global varaibles. Thus, engine->Put statements only have an effect on the 
  // root processor of the communicator defined by the injection point. 
  engine->Put("time", t);


  // To put a vector, users can use the Put_Vector function. Put_Vector assumes a global vector indexed by the rank of the processor 
  // within the injection points communicator. For instance, engine->Put_Vector("r",{rank}) would create a single vector {0,1,2...,n} 
  // where **n** is the number of processors in the vector. Put_Vector requires all vectors have the same size across processors. 
  engine->Put_Vector("x", x);
  engine->Put_Vector("y", y);
  engine->Put_Vector("z", z);

  VnV_Debug(LineExample, "GEEEERER %s", "sdfsdf");

  return SUCCESS;
}


// Register the executable with VNV
INJECTION_EXECUTABLE(LineExample)

/**
 * VnV Examples: Line Charts
 * -------------------------
 * 
 * This application demonstrates how to render Line charts using the VnV 
 * report generation support.
 * 
 * In this example we plot a simple line plot using the Plotly interface. A number of different
 * line chart options are presented. 
 * 
 * The vnv plotly interface is a thin RST wrapper around the plotly javascript/json api. We support 
 * a large majority of the options available within that api. Function type parameters are not supported
 * at this time.
 * 
 * The Plotly Javascript API uses **traces** to allow users to define multiple series within a single chart. Each
 * **trace** is a json object containing the data and configuration parameters that should be applied to that 
 * particular data series.
 * 
 *  In the VnV API you define **traces** using the *:trace.<name>: <chart-type>* option. For example, 
 * in the following plot, the user has defined a trace called main that will be rendered as a scatter chart. You can then set options 
 * within that trace using options of the form *:<name>.<option>: <value>*. In second and third options of the chart below, we use the 
 * *main.x* and *main.y* options to set the **x** and **y** data for the scatter plot. Layout options are set using the layout.* set of options.
 * Layout options represent global options applied to the chart as a whole, independent of the options set for each individual trace.   
 * 
 * 
 * Users can use the VnV specific <vnv-jmes-path> command to inject into the traces and layout configuration. The VnV report generation
 * software uses regular expressions to replace all commands matching this pattern prior to rendering the chart. JMES Path is feature rich
 * JSON query language designed to allow users to quickly and efficiently query json objects. VnV uses a (very slightly) modified implementation
 * of the python jmespath reference implementation to enable jmespath queries to be used to query the VnV simulation data object. 
 * 
 */
INJECTION_OPTIONS(LineExample,"{}") {
  return NULL;
}

int main(int argc, char** argv) {

  MPI_Init(&argc,&argv);

  INJECTION_INITIALIZE(LineExample, &argc, &argv, (argc == 2) ? argv[1] : "./inputfiles/line.json");

  int rank,world;
  
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&world);

  std::vector<double> x,y,z;
  std::vector<std::vector<double>> w;
  double t = 1.44;

  double drank = rank + 1.00;
  for (int i = 0; i < 4; i++ ) {
     x.push_back(rank + i);
     y.push_back(rank*2 + i);
     z.push_back(rank*3 + i);
     w.push_back({drank,drank,drank,drank});
  }

 /**
   * 
   * @title Simple VnV Injection point
   * 
   * This is a simple VnV Injection point. There is not much more to say. 
   * 
   **/
  INJECTION_POINT("LineExample", VWORLD, "mainpoint", x, y, z, w, t);

  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
