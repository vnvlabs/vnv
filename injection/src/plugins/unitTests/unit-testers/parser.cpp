#ifndef _EuclideanError_H
#define _EuclideanError_H

#include "VnV.h"

#include <sstream>

INJECTION_UNITTEST(Demo) {
        int x = 10;
        int y = 10;
        std::map<std::string, bool> results;

        std::string xstr = std::to_string(x);
        std::string ystr = std::to_string(y);
        std::ostringstream t1,t2,t3;

        t1 << "x(" << x << ") + y(" << y << ") == 20 (" << x+y << ")";
        t2 << "x(" << x << ") - y(" << y << ") == 0 (" << x-y << ")";
        t3 << "x(" << x << ") != y(" << y << ")";

        results[t1.str()] = ( (x + y) == 20 );
        results[t2.str()] = ( (x - y) == 0 );
        results[t3.str()] = ( x == y );
        results["x == 10"] = (x==10);
        results["y == 10"] = (y==10);
        return results;
}


#endif
