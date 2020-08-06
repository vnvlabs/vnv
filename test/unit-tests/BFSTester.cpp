
#ifndef BFSTester_H
#define BFSTester_H

#include <sstream>

#include "VnV.h"
#include "base/Utilities.h"
#include "plugins/engines/debug/DebugEngineManager.h"
using namespace VnV;

/**
 * Breath First Search function
 * The breadth first search function is used in the Transformation API to automatically
 * transform variables between injection point parmeters and test parameters.
 */

INJECTION_UNITTEST(VnVTestRunner, BFSTester,1) {
  std::map<std::string, std::map<std::string, std::string>> graph;

  // Test: Pass in a from that is not in the map --> should throw.
  try {
    VnV::bfs(graph, "from", "to");
    TEST_ASSERT_EQUALS("FromNotInMap", false, true);
  } catch (...) {
    TEST_ASSERT_EQUALS("FromNotInMap", true, true);
  }
  /**
   * This test ensures the breadth first search algorithm returns an
   * empty set when the from node equals the to node. The test is
   * run using
   *
   * .. code-block:: cpp
   *    :linenos:
   *
   *    VnV::bfs(graph,"from","from").size() == 0
   */
  TEST_ASSERT_EQUALS("Empty Set When From == To ",
                     (0 == VnV::bfs(graph, "from", "from").size()), true);

  // Test Pass in a from that is in the map, but a to that is not.
  std::map<std::string, std::string> ss = {{"a", "1"}, {"b", "2"}};
  std::map<std::string, std::string> ss1 = {{"c", "3"}, {"d", "4"}};
  std::map<std::string, std::string> ss2 = {{"e", "1"}, {"f", "2"}};
  std::map<std::string, std::string> ss3 = {{"f", "1"}, {"h", "2"}};
  graph["from"] = ss;
  graph["a"] = ss1;
  graph["b"] = ss2;
  graph["e"] = ss3;
  // To not in graph, so should fail.
  try {
    VnV::bfs(graph, "from", "to");
    TEST_ASSERT_EQUALS("To not in set", false, true);
  } catch (...) {
    TEST_ASSERT_EQUALS("To not in set", true, true);
  }

  // Go from "from" to "a", a one level jump.
  try {
    auto x = VnV::bfs(graph, "from", "a");
    TEST_ASSERT_EQUALS(
        "One Level", (x.size() == 1 && x[0].first == "a" && x[0].second == "1"),
        true);
  } catch (...) {
    TEST_ASSERT_EQUALS("One Level", false, true);
  }

  // Go from "from" to "f" --> from->b->f
  try {
    auto x = VnV::bfs(graph, "from", "e");
    TEST_ASSERT_EQUALS(
        "Two Level", (x.size() == 2 && x[0].first == "b" && x[1].first == "e"),
        true);
  } catch (...) {
    TEST_ASSERT_EQUALS("Two Level", false, true);
  }

  // Add in some loops
  std::map<std::string, std::string> ss4 = {
      {"h", "10"}, {"b", "3"}, {"i", "44"}};
  graph["h"] = ss4;
  try {
    auto x = VnV::bfs(graph, "from", "i");
    TEST_ASSERT_EQUALS(
        "Loops",
        (x.size() == 4 && x[0].first == "b" && x[1].first == "e" &&
         x[2].first == "h" && x[3].first == "i"),
        true);
  } catch (...) {
    TEST_ASSERT_EQUALS("Loops", false, true);
  }
}

#endif
