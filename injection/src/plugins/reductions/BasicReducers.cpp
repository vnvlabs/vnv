#include "VnV.h"


INJECTION_REDUCER(VNVPACKAGENAME, sum, true) {
    // Communitive reducer that just takes the sum
    out->axpy(1,in);
    return out;
}
INJECTION_REDUCER(VNVPACKAGENAME, prod, true) {
    out->mult(in);
    return out;
}
INJECTION_REDUCER(VNVPACKAGENAME, min, true) {
    return  out->compare(in) < 0 ? out : in ;
}
INJECTION_REDUCER(VNVPACKAGENAME, max, true) {
    return out->compare(in) > 0 ? out : in ;
}
INJECTION_REDUCER(VNVPACKAGENAME, sqrsum, true) {
    in->mult(in);
    out->mult(out);
    out->axpy(1,in);
    return out;
}



