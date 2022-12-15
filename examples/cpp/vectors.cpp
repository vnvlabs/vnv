
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <utility>

#include "VnV.h"

using namespace VnV;



/**
 * @title Vectors and Matrices
 * 
 * This example deomnstrates VnV support for collecting Vector data across iterations. 
 */
INJECTION_EXECUTABLE(VectorsExample)


int main(int argc, char** argv) {

  
  INJECTION_INITIALIZE(VectorsExample, &argc, &argv, (argc == 2) ? argv[1] : "./inputfiles/vectors.json");

  
  // Lets make a matrix of size 10x10 with w[i,j] = i*10 + j
  std::vector<std::vector<double>> w;
  for (int i = 0; i < 10; i++) {
    w.push_back({});
    for (int j = 0; j < 10; j++) {
      w[i].push_back( i*10 + j ); 
    }
  }

  
  /**
   * @title PLOT SOLUTION Contour Plot Of the Solution 
   *
   * In this contour plot the x axis is the solution. The y 
   * axis is the time. So, this is a contour plot of the 1D 
   * solution against time. 
   *
   * .. vnv-plotly::
   *    :trace.main: contour
   *    :main.y: {{as_json(time)}}
   *    :main.z: {{as_json(matrix)}}
   *    :layout.title.text: Asgard Solution against time.
   *    :layout.yaxis.title.text: time
   *    :layout.xaxis.title.text: index
   *
   * .. vnv-plotly::
   *    :trace.main: contour
   *    :main.y: {{as_json(time)}}
   *    :main.z: {{as_json(vector)}}
   *    :layout.title.text: Asgard Solution against time.
   *    :layout.yaxis.title.text: time
   *    :layout.xaxis.title.text: index
   * 
   * 
   * .. vnv-plotly::
   *    :trace.main: contour
   *    :main.z: {{as_json(adapt)}}
   *    :layout.title.text: Asgard Solution against time.
   *    :layout.yaxis.title.text: time
   *    :layout.xaxis.title.text: index
   *
   *
   * .. vnv-animation::
   *   :start: 0
   *   :end: {{length(vector)}}
   *   :step: 1 
   *   :prefix: Testprefix
   *   :trace.main: scatter
   *   :trace.main1: scatter
   *   :main.y: {{vector[${i}]}}
   *   :main1.y: {{vector[${i}]}}
   *   :main1.xaxis: x2
   *   :main1.yaxis: y2
   *   :layout.title.text: Asgard Solution against time.
   *   :layout.yaxis.title.text: time
   *   :layout.xaxis.title.text: index
   *   :layout.yaxis.range: [0,{{vector[${i}][-1]}}]
   *   :layout.yaxis2.range: [0,{{vector[${i}][-1]}}]
   *   :layout.yaxis2.title.text: time aa
   *   :layout.xaxis2.title.text: index aa
   *   :layout.grid.rows: 1
   *   :layout.grid.columns: 2
   *   :layout.grid.pattern: independent
   *
   * .. vnv-animation::
   *   :values: {{vector}}
   *   :labels: {{`[0, 1,2,3,4,5,6,7,8,9]`}}
   *   :prefix: Testprefix
   *   :trace.main: scatter
   *   :main.y: ${i}
   *   :layout.title.text: Asgard Solution against time.
   *   :layout.yaxis.title.text: time
   *   :layout.xaxis.title.text: index
   *   :layout.yaxis.range: [0,100]
   *   
   */
  INJECTION_POINT(VectorsExample, VWORLD, mainpoint, VNV_CALLBACK {

      //VnV Provides two ways of writing matrices -- either as a single matrix,
      //or through multiple calls to engine->Put with the same name. 

      auto gsizes = std::make_pair(10,10);
      auto offsets = std::make_pair(0,0);
      data.engine->Put_Matrix("matrix", w, gsizes, offsets);

      // Vector of vectors version
      for (int i = 0; i < 10; i++ ) {
         data.engine->Put_Vector("vector", w[i]) ;
      }

      //Vector of vectors where the size grows each time and then shrinks.
      w[0].clear();
      for (int i = 0; i < 10; i++) {
        w[0].push_back(i+10);
        data.engine->Put_Vector("adapt", w[0]);
      }
      for (int i = 0; i < 10; i++ ) {
        w[0].pop_back();
        data.engine->Put_Vector("adapt", w[0]);
      }


  }, w);

  INJECTION_FINALIZE(SPNAME);

}
