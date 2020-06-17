#ifndef newtH
#define newtH

#include "VnV.h"

void nlnewt(void (*f)(double *x,double *fv,int n),double *x,
    double *fv,double **jac,double *p,int n,double eps);
void nlnewt2(void (*f)(double *x,double *fv,int n),double *x,
    double *fv,int n,double eps,int iter);
void nlnewt3(void (*f)(double *x,double *fv,int n),double *x,
    double *fv,int n,double eps,int *maxiter);


#endif
