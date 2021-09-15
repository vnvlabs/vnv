
#include <iostream>

/** Include the VnV include file **/
#include "VnV.h"

// Reducers are functions that can be used with a
// MPI_Reduce call. VnV will

// Inside your reduction you get to DataType_ptr, in and out.
// The goal of the reduction function is to combine those ptrs
// in some way and store the result in out.

// In this example we have out = out + in;
// The last parameter indicates if the reduction is cummulative. (A+B == B+A)
// There are more opportunities for optimization in cummulative reduction
// operations.

INJECTION_REDUCER(Samples, custom_reduction, true) {
  // Communitive reducer that just takes the sum
  out->axpy(1, in);
  return out;
}