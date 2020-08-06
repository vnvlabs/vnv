#ifndef _EuclideanError_H
#define _EuclideanError_H

#include "VnV.h"
#include <sstream>

#include "interfaces/IUnitTest.h"
#include "plugins/engines/ParallelOutputEngine/SparseBitVector.h"

using namespace VnV;

class SparseBitVectorUnitTests : public IUnitTest {
public:

   SparseBitVectorUnitTests() {}

   void run() {
        try {
            testCreate();
            testIsEmpty();
            testString();
            testOperator();
        } catch (std::exception& e) {
            // just return results for now
        }
    }

    ~SparseBitVectorUnitTests();

private:
    void testCreate() {
        SparseBitVector b(23);
        TEST_ASSERT_EQUALS("create1", b.empty(), false);
        TEST_ASSERT_EQUALS("create2", b.test(23), true);
        b.reset(23);
        TEST_ASSERT_EQUALS("create3", b.empty(), true);
    }

    void testIsEmpty() {
        SparseBitVector s1;
        TEST_ASSERT_EQUALS("is_empty1", s1.empty(), true);
        s1.set(10);
        TEST_ASSERT_EQUALS("is_empty2", s1.empty(), false);
        for (int i = 200000; i < 300000; i+=2) {
            s1.set(i);
        }
        s1.reset(10);
        for (int i = 200000; i < 300000; i+=2) {
            s1.reset(i);
        }
        TEST_ASSERT_EQUALS("is_empty3", s1.empty(), true);
     }

    void testString() {
        SparseBitVector s1;
        SparseBitVector s2;
        std::string str;
        for (int i = 200000; i < 300000; i++) {
            s1.set(i);
        }
        str = s1.toString();
        // Set a random bit to make sure it is cleared.
        TEST_ASSERT_EQUALS("tostring1", "[200000-299999]", str);
   }

   void testOperator() {
       SparseBitVector s1;
       SparseBitVector s2;

       for (int i = 0 ; i < 100; i += 2) {
           s1.set(i);
       }
       for (int i = 1 ; i < 100; i += 2) {
           s2.set(i);
       }

       s1 |= s2;

       TEST_ASSERT_EQUALS("|= (count)", 100, s1.count());
       TEST_ASSERT_EQUALS("|= (string)", "[0-99]", s1.toString());
   }
};

SparseBitVectorUnitTests::~SparseBitVectorUnitTests(){}

/**
 * Unit tests for the sparse bit vector code.
 */
INJECTION_UNITTEST_RAW(VnVTestRunner,bitvector,SparseBitVectorUnitTests,1)

#endif
