#ifndef _EuclideanError_H
#define _EuclideanError_H

#include "VnV.h"
#include <sstream>


INJECTION_UNITTEST(Demo) {
        int x = 10;
        int y = 10;

        std::string xstr = std::to_string(x);
        std::string ystr = std::to_string(y);
        std::ostringstream t1,t2,t3;

        t1 << "x(" << x << ") + y(" << y << ") == 20 (" << x+y << ")";
        t2 << "x(" << x << ") - y(" << y << ") == 0 (" << x-y << ")";
        t3 << "x(" << x << ") != y(" << y << ")";

        TEST_ASSERT_EQUALS(t1.str(), 20, x + y);

        TEST_ASSERT_EQUALS(t2.str(), 0, x - y);

        TEST_ASSERT_EQUALS(t3.str(), x, y);

        TEST_ASSERT_EQUALS("x == 10", 10, x);

        TEST_ASSERT_EQUALS("y == 10", 10, y);
}



#endif
