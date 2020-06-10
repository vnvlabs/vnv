#ifndef _ROUTERUNITTEST_H
#define _ROUTERUNITTEST_H

#include "VnV.h"
#include <iostream>
#include <time.h>

#include "plugins/engines/ParallelOutputEngine/Router.h"

using namespace VnV;

#define MAX_ITERS 1000

class RouterUnitTests : public IUnitTest {
public:

    RouterUnitTests() {srand(time(0));}

   void run() {
        try {
            testTree();
        } catch (std::exception& e) {
            // just return results for now
        }
    }

    ~RouterUnitTests();

private:
    void testTree() {
        int size = rand() % 100 + 100;
        int id = rand() % size;
        int root = rand() % size;
        int fanout = rand() % 5 + 3;
        Router root_router(root, root, size, fanout);
        std::cout << "id=" << id << ", root=" << root << ", size=" << size << ", fanout=" << fanout << std::endl;

        // Check the parent of root is root
        TEST_ASSERT_EQUALS("root parent", root, root_router.parent_of());

        // Check the parent of all children of root is root
        for (auto it : root_router.children_of()) {
            Router router(it, root, size, fanout);
            TEST_ASSERT_EQUALS("parent_of_" + std::to_string(it), root, router.parent_of());
        }

        // Check the root of an arbitrary node is root
        Router node(id, root, size, fanout);
        int parent = node.parent_of();
        int cnt = MAX_ITERS; // for safety
        while (parent != id && cnt-- > 0) {
            Router parent_router(parent, root, size, fanout);
            id = parent;
            parent = parent_router.parent_of();
        }
        TEST_ASSERT_NOT_EQUALS("root_of", 0, cnt);
        TEST_ASSERT_EQUALS("root_of", root, parent);
    }
};

RouterUnitTests::~RouterUnitTests(){}


/**
 * Unit Tests for the Router Class of the Parallel Output Engine.
 * .. vnv-results-table:: $.results
 */
INJECTION_UNITTEST_RAW(router, RouterUnitTests,1)



#endif
