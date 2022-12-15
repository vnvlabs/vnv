
#include <mpi.h>

#include <iostream>
#include <vector>

#include "VnV.h"
#define SPNAME Distributed

INJECTION_EXECUTABLE(SPNAME)

/*
 *  This test writes a distributed vector to file. It requires a
 * std::vector<double> x to run. The input file allows you to map injection
 * point parameters to test parameters.
 *
 *  Lets plot the vector using the chart library.
 *
 *  .. vnv-chart::
 *     :ydata: Data.Vector1.Value
 *
 *     {
 *       "type" : "line",
 *       "data" : {
 *          "labels" : $$ydata$$,
 *          "datasets" : [{
 *             "label": "Recorded CPU Times",
 *             "backgroundColor": "rgb(255, 99, 132)",
 *             "borderColor": "rgb(255, 99, 132)",
 *             "data": $$ydata$$
 *           }]
 *       }
 *     }
 */
INJECTION_TEST(SPNAME, write_vec) {
  std::string var = m_config.getAdditionalParameters().value("var","local_vec");
  auto x = GetRef(var,std::vector<double>);

  // Write a vector indexed by the rank with local size x.size() on each
  // process.
  engine->Put_Vector("Vector1", x);

  // Write a vector using the raw data. In this case we set the local size
  //  to 2. The final vector written will be world_size * 2
  engine->Put_Vector("Vector2", x.size(), x.data());

  // Write using reference to a single element
  engine->Put_Vector("Vector3", x[0]);
  return SUCCESS;
}

/**
 * Put_Rank Test examples.
 * -----------------------
 *
 * This test demos the Put_Rank methods. These methods write vector
 * data from ONE process with rank = r.
 *
 * template<typename T>
 * auto& x() { return getReference<std::vector<double> x>("std::vector<double>
 * x");
 */
INJECTION_TEST(SPNAME, put_rank) {

  std::string var = m_config.getAdditionalParameters().value("var","local_vec");
  auto x = GetRef(var,std::vector<double>);

  // Could also pull the rank from the user options (TODO).
  int rank = 0;

  // Write a vector indexed by the rank with local size x.size() on each
  // process.
  engine->Put_Rank("Vector1", x, rank);

  // Write a vector using the raw data. In this case we set the local size
  engine->Put_Rank("Vector2", x.size(), x.data(), rank);

  // Write using reference to a single element
  engine->Put_Rank("Vector3", x[0], rank);

  return SUCCESS;
}

/**
 * Put_Matrix Test examples.
 * -----------------------
 *
 * This test demos the Put_Matrix methods. These methods write vector
 * data from ONE process with rank = r.
 *
 * TODO/Questions
 * Do we want a standard VnV Way to define domain decomposition for
 * a matrix. This test only works because I know the matrix is row-wise
 * More information is needed to generate more data.
 *
 */
INJECTION_TEST(SPNAME, put_matrix) {

  std::string var = m_config.getAdditionalParameters().value("var","local_mat");
  auto x = GetRef(var,std::vector<std::vector<double>>);

  // First one passed in data + cols where cols in the number of columns in
  // the global output matrix. This function will take and [x,y] local matrix
  // and map it to a block matrix with "cols" columns. For a rowwise matrix we
  // use cols = x[0].size() to represet the length of row 0.
  engine->Put_Matrix("Matrix1", x, x[0].size());

  // Second one is a generic matrix. this one allows for different local sizes.
  //  and non rank based ordering.
  std::pair<int, int> gsize =
      std::make_pair(x.size() * comm->Size(), x[0].size());
  std::pair<int, int> offs = std::make_pair(x.size() * comm->Rank(), 0);
  engine->Put_Matrix("Matrix2", x, gsize, offs);
  return SUCCESS;
}

/**
 * Reduction Demonstration
 * -----------------------
 *
 * Demonstrates the reduction interface. These API functions allow
 * a vector to be reduced across the communicator.
 */
INJECTION_TEST(SPNAME, put_reduce) {

  std::string var = m_config.getAdditionalParameters().value("var","local_vec");
  auto x = GetRef(var,std::vector<double>);

  // Find the "min" value in a global vector.
  engine->Put_ReduceVector("Reduce1", "VNV:max", x);

  // Reduce a scalar across all process. This finds the max of x[1]
  // across all processes.
  engine->Put_ReduceScalar("Reduce2", "VNV:max", x[1]);

  // Reduce the vector element wise == produces a vector of size x.size()
  engine->Put_ReduceVectorElementWise("Reduce3", "VNV:sum", x);

  // Reduce the vector on every process and write a vector of length comm_size
  engine->Put_ReduceVectorRankWise("Reduce4", "VNV:sum", x);

  // Reduce the vector on rank 0 and use that as the result
  engine->Put_ReduceVectorRankOnly("Reduce5", "VNV:sum", x, 0);

  // Run a custom reduction on a global vector
  engine->Put_ReduceVector("Reduce6", "Distributed:custom_reduction", x);
  return SUCCESS;
}

INJECTION_REDUCER(SPNAME, custom_reduction, true) {
  // Communitive reducer that just takes the sum
  out->axpy(5, in);
  return out;
}

void cust(void* in, void* inout, int* len, MPI_Datatype* data) {
  auto d = (double*)inout;
  auto di = (double*)inout;
  d[0] += di[0];
  d[1] += di[1];
}

typedef struct _options_struct {
  int lsize = 10;
  int gsize = 3;
} options_struct;
options_struct options;

INJECTION_OPTIONS(SPNAME, R"(
  {
     "type" : "object",
     "properties" : {
        "lsize" : {"type" : "integer" },
        "vsize" : {"type" : "integer" }
     }
  }
)",void) {
  if (config.contains("lsize")) {
    options.lsize = config["lsize"].get<int>();
  }
  if (config.contains("vsize")) {
    options.gsize = config["vsize"].get<int>();
  }
  return NULL;
}

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /**
   * Distributed Data Example
   * ------------------------
   *
   * This executable demonstrated the distributed data capabilities of the
   * IOutputEngine interface. The example provides a single injection point with
   * a vector of doubles as the only available input parameter.
   *
   * In this example, we provide several VnV tests demoing the distributed data
   * support. These test expect a vector of doubles as input and write data to
   * using the various distributed data functions.
   *
   * The example expects a file called "vv-dist-data.json"  to be available in
   * the directory it is run from.
   *
   * The executable expects two command line arguements. The first represents
   * the local size of the vector on each process. The second parameter
   * represents the size of the matrix on each local process for the matrix
   * examples.
   *
   * For example mpirun -n p <exe> 10 3 will create a vector of length p*size
   * and a matrix of size [3*p,3*p]
   *
   *
   * .. hint::
   *    This comment will show up in the final report as the introduction. It
   * supports restructured text markup.
   */
  INJECTION_INITIALIZE(SPNAME, &argc, &argv, "./inputfiles/distributed.json");

  // Assign the global vector. This is a vector of "doubles" where
  // the values is g[i] = i. The vector is distributed across the processes
  // so each process owns the range [rank*local_vec_size,
  // rank*(local_vec_size+1))
  int local_vec_size = options.lsize;
  std::vector<double> local_vec(local_vec_size);
  std::iota(local_vec.begin(), local_vec.end(), rank * local_vec_size);

  // Assign the global matrix. This is a row wise data decomposition
  // Each process owns complete rows of the matrix. It owns local_mat_size
  // rows with size*local_mat_size elements in each row. The values are
  // are top down numbering of the matrix Aij = i*local_mat_size + j
  int local_mat_size = options.gsize;
  int global_mat_dim = size * local_mat_size;
  std::vector<std::vector<double>> local_mat(local_mat_size);
  for (int i = 0; i < local_mat_size; i++) {
    local_mat[i].resize(size * local_mat_size);
    std::iota(local_mat[i].begin(), local_mat[i].end(),
              (i + rank * local_mat_size) * size * local_mat_size);
  }

  /*
   *  Distributed Data Injection Point: Vector Example.
   *  -------------------------------------------------
   *
   *  This injection point provides access to a distributed
   *  vector and matrix.
   *
   */
  INJECTION_POINT(SPNAME, VWORLD, distribute, VNV_NOCALLBACK, local_vec, local_mat);

  /**
     Conclusion.
     ===========

     Whatever we put here will end up in the conclusion.
  */
  INJECTION_FINALIZE(SPNAME);

  MPI_Finalize();
}
