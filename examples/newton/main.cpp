
#include "newt.h"

void nonlinear(double* x, double* fv, int size) {}

void nlnewt3(void (*f)(double* x, double* fv, int n), double* x, double* fv,
             int n, double eps, int* maxiter);

INJECTION_EXECUTABLE(Newton)

int main(int argc, char** argv) {
  INJECTION_INITIALIZE(Newton, &argc, &argv,
                       (argc == 2) ? argv[1] : "./newton-input.json");

  VnV_Debug(Newton, "Up and running");

  int maxit = 100;
  double eps = 1e-8;
  int n = 100;

  double x[n];
  double fv[n];

  for (int i = 0; i < n; i++) {
    x[i] = (double)rand() / RAND_MAX;
  }

  nlnewt3(&nonlinear, x, fv, n, eps, &maxit);

  INJECTION_FINALIZE(Newton)

  return 0;
}
