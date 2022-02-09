
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
   * @title Contour Plot For a Shape Variable (as a third party test)
   * 
   * In this example we plot a contour plot for a shape parameter. This
   * is an example where you have all the data on a single iteration.
   * 
   * .. vnv-plotly::
   *    :trace.main: contour
   *    :main.x: {{as_json(x[0])}}
   *    :main.y: {{as_json(y[0])}}
   *    :main.z: {{as_json(z[0])}}
   *    :layout.title.text: Basic contour plot
   * 
   **/
INJECTION_TEST(ContourExample, contour) {
  auto x = GetRef_NoCheck("x", std::vector<double>);
  auto y = GetRef_NoCheck("y", std::vector<double>);
  auto z = GetRef_NoCheck("z", std::vector<std::vector<double>>);

  engine->Put_Vector("x", x);
  engine->Put_Vector("y", y);
  
  auto gsizes = std::make_pair(comm->Size()*4,comm->Size()*4);
  auto offsets = std::make_pair(comm->Rank()*4,comm->Rank()*4);
  engine->Put_Matrix("z", z, gsizes, offsets);
  return SUCCESS;
}


// Register the executable with VNV
INJECTION_EXECUTABLE(ContourExample)

int main(int argc, char** argv) {

  MPI_Init(&argc,&argv);

  INJECTION_INITIALIZE(ContourExample, &argc, &argv, (argc == 2) ? argv[1] : "./inputfiles/contour.json");

  int rank,world;
  
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&world);

  std::vector<double> x,y;
  std::vector<std::vector<double>> z;
  
  double drank = rank + 1.00;
  for (int i = 0; i < 4; i++ ) {
     x.push_back(rank*4 + i);
     y.push_back(rank*4 + i);
     z.push_back({drank,drank,drank,drank});
  }

 /**
   * 
   * @title Contour Plot For a Shape Variable (as a callback)
   * 
   * In this example we plot a contour plot for a shape parameter. This
   * is an example where you have all the data on a single iteration.
   * 
   * .. vnv-plotly::
   *    :trace.main: contour
   *    :main.x: {{as_json(xx[0])}}
   *    :main.y: {{as_json(yy[0])}}
   *    :main.z: {{as_json(zz[0])}}
   *    :layout.title.text: Basic contour plot
   * 
   **/
  INJECTION_POINT_C("ContourExample", VWORLD, "mainpoint", IPCALLBACK {
      engine->Put_Vector("xx", x);
      engine->Put_Vector("yy", y);

      auto gsizes = std::make_pair(world*4,world*4);
      auto offsets = std::make_pair(rank*4,rank*4);
      engine->Put_Matrix("zz", z, gsizes, offsets);
  }, x, y, z);

  /**
   * @title Contour Plot For a Standard Vector Iterated (as a callback)
   *
   * In this example we add a row to the z parameter on every iteration.
   * This will build up a contour plot from a set of vectors. Also note
   * we leave out x and y values for data. This will plot values based
   * on the index.
   *
   * .. vnv-plotly::
   *    :trace.main: contour
   *    :main.z: {{as_json(x)}}
   *    :layout.title.text: Basic contour plot
   *
   */
  INJECTION_LOOP_BEGIN_C("ContourExample", VWORLD, "mainloop", IPCALLBACK {
      engine->Put_Vector("x",x);
  }, x);
  
  for (double i = rank; i < rank + 5; i++ ) {
      x = {i, i*i, i*i*i , i*i*i*i };
      INJECTION_LOOP_ITER("ContourExample", "mainloop", "inner");
  }
  INJECTION_LOOP_END("ContourExample", "mainloop");


  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
