#include "base/communication/CommMapper.h"

#include <unistd.h>

#include <iostream>
#include <memory>
#include <set>

#include "VnV.h"
#include "base/communication/Communication.h"
#include "base/communication/ICommunicator.h"
#include "validate/json-schema.hpp"
#include "shared/exceptions.h"
using namespace VnV;
using nlohmann::json;
namespace {

// Forward declare add
void add(long proc, CommWrap_ptr node, CommMap& willAdd, CommWrap_ptr newNode);

void swap(long proc, CommWrap_ptr node, CommMap& willAdd, CommWrap_ptr newNode) {
  if (node->parents.size() == 1) {
    auto parent = node->parents.begin()->second;
    if (willAdd.find(parent->id) != willAdd.end()) {
      // add(proc, parent, willAdd, newNode);
    } else if (parent->contents.size() == node->contents.size()) {
      // Nodes parents are now parents parents. Also tell parents parents
      // that they are now linklinked to Node instead of parent.
      node->parents.clear();
      for (auto p : parent->parents) {
        node->parents.insert(std::make_pair(p.first, p.second));
        p.second->children.erase(parent->id);
        p.second->children.insert(std::make_pair(node->id, node));
      }
      // Parents parent is now node.
      parent->parents.clear();
      parent->parents.insert(std::make_pair(node->id, node));

      // Parents Children are now Nodes children
      parent->children.clear();
      for (auto c : node->children) {
        parent->children.insert(std::make_pair(c.first, c.second));
        c.second->parents.erase(node->id);
        c.second->parents.insert(std::make_pair(parent->id, parent));
      }
      // Nodes children is now parent.
      node->children.clear();
      node->children.insert(std::make_pair(parent->id, parent));

      // Try swap again to see if i need to swap again.
      swap(proc, node, willAdd, newNode);
    }
  }
}

void add(long proc, CommWrap_ptr node, CommMap& willAdd, CommWrap_ptr newNode) {
  // maknode->toJson();

  // This checks for a special case where the parent is also the same
  // as the child currently.

  swap(proc, node, willAdd, newNode);

  std::vector<CommWrap_ptr> toDelete;

  for (auto& parent : node->parents) {
    if (parent.second->contents.find(proc) != parent.second->contents.end()) {
      // Parent already added this proc --> We are safe to add this proc based
      // on this parent. Note that we should remove newNode from parents
      // children because I will add it as my child (later on)
      if (newNode != nullptr) {
        parent.second->children.erase(newNode->id);  // Erase New Node.
        newNode->parents.erase(parent.second->id);
      }

    } else if (willAdd.size() > 0 && willAdd.find(parent.second->id) != willAdd.end()) {
      for (auto it : willAdd) {
      }

      // Add the proc to the parent, then I am done with this parent.
      add(proc, parent.second, willAdd, newNode);
      if (newNode != nullptr) {
        parent.second->children.erase(newNode->id);  // Erase New Node.
        newNode->parents.erase(parent.second->id);
      }
    } else {
      toDelete.push_back(parent.second);
    }
  }

  if (toDelete.size() == 0) {
    // Added so we can remove from the will add list.
    // Add the new Node as a child of this node. Any children that also
    // include this processor will erase this link if need be.

    node->contents.insert(proc);
    if (newNode != nullptr) {
      node->children.insert(std::make_pair(newNode->id, newNode));
      newNode->parents.insert(std::make_pair(node->id, node));
    }
    willAdd.erase(node->id);

  } else {
    for (auto parent : toDelete) {
      // Parent is not adding this node, so I am no longer a child of this
      // parent.
      node->parents.erase(parent->id);   // parent is no longer my parent.
      parent->children.erase(node->id);  // i am no longer a child of this parent.

      // Parents parents are now my parents (and still parents parents).
      for (auto it : parent->parents) {
        node->parents.insert(std::make_pair(it.first, it.second));
        it.second->children.insert(std::make_pair(node->id, node));
      }

      // My children are now my parents children (and still my children).
      for (auto it : node->children) {
        parent->children.insert(std::make_pair(it.first, it.second));
        it.second->parents.insert(std::make_pair(parent->id, parent));
      }
    }

    // Try again with my new parent list.
    add(proc, node, willAdd, newNode);
  }
}

void add(long proc, CommMap& oldNodes, CommWrap_ptr newNode) {
  while (oldNodes.size() > 0) {
    add(proc, oldNodes.begin()->second, oldNodes, newNode);
  }
}

void appendToCommMap(CommMap& m, std::vector<long>& comms, std::vector<int>& counts) {
  int c = 0;
  for (int proc = 0; proc < counts.size(); proc++) {
    CommWrap_ptr firstNewNode = nullptr;
    CommWrap_ptr lastNewNode = nullptr;
    CommMap oldNodes;
    for (int comm = 0; comm < counts[proc]; comm++) {
      long comVal = comms[c++];
      auto commWrap_iter = m.find(comVal);
      if (commWrap_iter == m.end()) {
        // New nodes are just chained together, one after the other. At the end,
        // the first node in this chain will be attached to any existing node
        // that contains this processor, but does has no children with this
        // processor.
        auto w = std::make_shared<CommWrap>(comVal);
        w->contents.insert(proc);
        if (lastNewNode != nullptr) {
          lastNewNode->children.insert(std::make_pair(w->id, w));
          w->parents.insert(std::make_pair(lastNewNode->id, lastNewNode));
        }
        lastNewNode = w;

        // Later, first new node should be added as a child to any node
        // that contains <proc> but has no children that contain <proc>
        if (firstNewNode == nullptr) {
          firstNewNode = w;
        }
        // Add it to the full map.
        m.insert(std::make_pair(w->id, w));
      } else {
        oldNodes.insert({commWrap_iter->first, commWrap_iter->second});
      }
    }
    add(proc, oldNodes, firstNewNode);
  }
}

std::set<CommWrap_ptr> getCommMap(std::vector<long>& comms, std::vector<int>& counts) {
  CommMap m;
  appendToCommMap(m, comms, counts);

  std::set<CommWrap_ptr> rootComms;
  for (auto it = m.begin(); it != m.end(); it++) {
    CommWrap_ptr me = it->second;

    if (me->parents.size() == 0 && me->contents.size() > 0) {
      rootComms.insert(it->second);  // I have no parents so I must be a root communicator.
    }
  }
  return rootComms;
}

}  // namespace

long CommMapper::getNextId(ICommunicator_ptr comm, long myVal) {
  long recv = 0;

  if (myVal > id) id = myVal;

  comm->AllReduce(&id, 1, &recv, sizeof(long), VnV::OpType::MAX);
  id = recv + 1;

  return recv;
}

bool CommMapper::isNew(ICommunicator_ptr comm) { return comms.find(comm->uniqueId()) == comms.end(); }

void CommMapper::logComm(ICommunicator_ptr comm) {
  long id = comm->uniqueId();
  comms.insert(id);
  if (comm->Rank() == 0) {
    rootComms.insert(id);
  }
}

template <typename T>
std::pair<std::vector<T>, std::vector<int>> gatherSet(std::set<T>& data, ICommunicator_ptr worldComm, bool root) {
  // First do a Gather to determine the number of comms on each process.
  int send = data.size();
  std::vector<int> recv(worldComm->Rank() == root ? worldComm->Size() : 1);

  worldComm->Gather(&send, 1, recv.data(), sizeof(int), root);

  // Now a all gatherV
  int recv2Coun = 0;
  int prev = 0;
  std::vector<int> displs(worldComm->Size());
  if (worldComm->Rank() == root) {
    displs[0] = 0;
    for (int i = 1; i < worldComm->Size(); i++) {
      displs[i] = displs[i - 1] + recv[i - 1];
    }
    recv2Coun = displs.back() + recv.back();
  }
  std::vector<long> recv2(recv2Coun);
  std::vector<long> commVec(data.begin(), data.end());
  worldComm->GatherV(commVec.data(), commVec.size(), recv2.data(), recv.data(), displs.data(), sizeof(long), 0);
  return std::make_pair(recv2, recv);
}

std::set<CommWrap_ptr> CommMapper::gatherCommInformation(ICommunicator_ptr worldComm) {
  auto a = gatherSet<long>(comms, worldComm, root);
  if (worldComm->Rank() == root) {
    return getCommMap(a.first, a.second);
  }
  return {};
}

DynamicCommMap::DynamicCommMap(int worldSize) { worldsize = worldSize; }

void DynamicCommMap::append(long commId, std::vector<int> procs) {
  // We have a list of procs involved in a communicator. We want to append them
  // to the comm map.

  // Unfortuantly, appendToCommMap was written to determine comms in a
  // distributed setting. Basically, it is designed to build a comm map in cases
  // where the distribution is not known. In our case, we have a
  //  comm id and a list of included procs. In there case, we have a list of
  //  comms that contain each proc. In other words, this needs to be rewritten
  //  at some point. For now I am interfacing to the old version to get things
  //  going.

  // FIXME FIXME FIXME -- I havn't compiled in three days -- This is the last
  // thing stopping me --

  // Procs.size() == 0 is the world comm.
  std::vector<long> recv(procs.size() == 0 ? worldsize : procs.size(), commId);
  std::vector<int> counts(worldsize,
                          procs.size() == 0 ? 1 : 0);  // The number of
  for (auto& it : procs) {
    counts[it] = 1;
  }
  appendToCommMap(map, recv, counts);
}

CommWrap_ptr DynamicCommMap::getRootCommunicator() {
  for (auto it = map.begin(); it != map.end(); it++) {
    CommWrap_ptr& me = it->second;
    if (me->parents.size() == 0 && me->contents.size() > 0) {
      return it->second;
    }
  }
  return nullptr;
}

bool DynamicCommMap::searchNodeInMainNodeCommChain(long mainNode, long searchNode) {
  // Need to find out if search node and main node have any intersection.
  auto it = map.find(mainNode);
  if (it != map.end()) {
    return it->second->inCommChain(searchNode);
  }
  throw INJECTION_EXCEPTION("unregistered communicator %d in %d", mainNode, searchNode);
}

std::vector<long> CommMapper::listAllComms(ICommunicator_ptr worldComm) {
  auto a = gatherSet<long>(rootComms, worldComm, root);
  if (worldComm->Rank() == root) {
    // a.first is a vector containing all the root comms. In theory, each comm
    // has a single root, and every comm is unique, so....., this should be a
    // valid list as is.
    return a.first;
  }
  return {};
}

std::shared_ptr<InjectionPointMerger> InjectionPointMerger::join(
      std::string outputfile, std::set<CommWrap_ptr>& comms,
      std::function<std::shared_ptr<InjectionPointInterface>(long)>& parse) {
    INJECTION_ASSERT((comms.size() != 1), "Invalid Comms Object. Size should be one but it was %ld", comms.size());

    std::map<long, CommWrap_ptr> commsMap = CommMapper::convertToMap(comms);
    std::shared_ptr<InjectionPointMerger> dstruct = nullptr;
    std::set<long> done;
    for (auto it : commsMap) {
      join(dstruct, it.second->id, commsMap, outputfile, done, parse);
    }
    return dstruct;
  }


long CommMapper::id = 0;

/**
 * Communication Mapper Testing
 * ============================
 *
 * The CommMapper class is included in the VnV toolkit as a built in way to
 * track communication patterns in parallel applications.
 *
 */
INJECTION_UNITTEST(VNVPACKAGENAME, CommMapTester, 1) {
  /**
   * Does the comm-map tester compile. This test never fails.
   */
  TEST_ASSERT_EQUALS("Compiles", true, true);

  std::shared_ptr<CommMapper> p;
  bool tryTest1 = false;
  try {
    p.reset(new CommMapper());
    tryTest1 = true;
  } catch (std::exception& e) {
  };
  /**
   * Does Constructor work
   */
  TEST_ASSERT_EQUALS("Constructor", true, tryTest1);
  if (!tryTest1) return;  // Cant continue if failed.

  /**
   * Can we add a communicator and get an id.
   */
  p->logComm(comm);
  
  /**
   * After Adding a communicator was the comms set property. This test
   * ensures that logging a comm adds it to the list of registered comms.
   *
   */
  TEST_ASSERT_EQUALS("AddComm", true, p->comms.size() == 1 && *(p->comms.begin()) == comm->uniqueId());

  // Call log comm again.
  p->logComm(comm);

  /**
   * Adding the same comm again should do nothing. This is all but guaranteed
   * assuming comm returns the same id.
   */
  TEST_ASSERT_EQUALS("Add Same Comm Twice", true, p->comms.size() == 1);

  std::vector<long> comms1{10, 0, 10, 1};
  std::vector<int> counts1 = {2, 2};
  std::set<CommWrap_ptr> pp1 = getCommMap(comms1, counts1);
  json exact =
      R"({"0":{"c":[0],"ch":[],"p":[10]},"1":{"c":[1],"ch":[],"p":[10]},"10":{"c":[0,1],"ch":[0,1],"p":[]}})"_json;

  /**
   * Test 1:
   * -------
   *
   * Processor 0: World, Self
   * Processor 1: World, Self
   *
   * The expected Result is:
   *
   * .. vnv-jchart::
   *
   *    {"0":{"c":[0],"ch":[],"p":[10]},"1":{"c":[1],"ch":[],"p":[10]},"10":{"c":[0,1],"ch":[0,1],"p":[]}}
   *
   */
  TEST_ASSERT_EQUALS("Gold Test One", true, pp1.size() == 1 && json::diff(exact, (*pp1.begin())->toJson()).size() == 0)

  std::vector<long> comms{100, 10, 20, 0, 100, 10, 30, 1, 100, 20, 30, 2};
  std::vector<int> counts = {4, 4, 4};
  std::set<CommWrap_ptr> pp = getCommMap(comms, counts);
  json exactR =
      R"({"0":{"c":[0],"ch":[],"p":[10,20]},"1":{"c":[1],"ch":[],"p":[10,30]},"10":{"c":[0,1],"ch":[0,1],"p":[100]},"100":{"c":[0,1,2],"ch":[10,20,30],"p":[]},"2":{"c":[2],"ch":[],"p":[20,30]},"20":{"c":[0,2],"ch":[0,2],"p":[100]},"30":{"c":[1,2],"ch":[1,2],"p":[100]}})"_json;

  /**
   * Test 2:
   * -------
   *
   * Processor 0: World, Self, {0,1}, {0,2}
   * Processor 1: World, Self, {0,1}, {1,2}
   * Processor 2: World, Self, {0,2}, {1,2}
   *
   * The expected Result is:
   *
   * .. vnv-jchart::
   *
   *   {"0":{"c":[0],"ch":[],"p":[10,20]},"1":{"c":[1],"ch":[],"p":[10,30]},"10":{"c":[0,1],"ch":[0,1],"p":[100]},"100":{"c":[0,1,2],"ch":[10,20,30],"p":[]},"2":{"c":[2],"ch":[],"p":[20,30]},"20":{"c":[0,2],"ch":[0,2],"p":[100]},"30":{"c":[1,2],"ch":[1,2],"p":[100]}}
   *
   */
  TEST_ASSERT_EQUALS("God Test Two", true, pp.size() == 1 && json::diff(exactR, (*pp.begin())->toJson()).size() == 0)

  comms = {0};
  counts = {1};
  json exactSingle = R"({"0":{"c":[0], "ch":[],"p":[]}})"_json;
  pp = getCommMap(comms, counts);
  /**
   * Test 3:
   *
   * Processor 0: World
   *
   * The expected result is:
   *
   * .. vnv-jchart::
   *
   *   {"0":{"c":[0], "ch":[],"p":[]}}
   *
   */
  TEST_ASSERT_EQUALS("Gold Test Three", true,
                     pp.size() == 1 && json::diff(exactSingle, (*pp.begin())->toJson()).size() == 0)

  comms = {100, 0, 100, 1, 100, 2, 100, 3, 100, 4, 100, 5};
  counts = {2, 2, 2, 2, 2, 2};
  exactSingle =
      R"({"100": {"c":[0,1,2,3,4,5], "ch":[0,1,2,3,4,5], "p":[]},"0":{"c":[0], "ch":[],"p":[100]},"1":{"c":[1], "ch":[],"p":[100]},"2":{"c":[2], "ch":[],"p":[100]},"3":{"c":[3], "ch":[],"p":[100]},"4":{"c":[4], "ch":[],"p":[100]},"5":{"c":[5], "ch":[],"p":[100]}})"_json;
  pp = getCommMap(comms, counts);
  /**
   * Test 4:
   *
   * Processor 0-5: World, SELF
   *
   * The expected result is:
   *
   * .. vnv-jchart::
   *
   *  {"100": {"c":[0,1,2,3,4,5], "ch":[0,1,2,3,4,5], "p":[]},"0":{"c":[0],
   * "ch":[],"p":[100]},"1":{"c":[1], "ch":[],"p":[100]},"2":{"c":[2],
   * "ch":[],"p":[100]},"3":{"c":[3], "ch":[],"p":[100]},"4":{"c":[4],
   * "ch":[],"p":[100]},"5":{"c":[5], "ch":[],"p":[100]}}
   *
   */
  TEST_ASSERT_EQUALS("Gold Test Four", true,
                     pp.size() == 1 && json::diff(exactSingle, (*pp.begin())->toJson()).size() == 0)
}

INJECTION_UNITTEST(VNVPACKAGENAME, CommTestTwoProc, 2) {
  CommMapper m;
  m.logComm(comm);
  m.logComm(comm->self());
  auto pp = m.gatherCommInformation(comm);

  json exactR =
      R"({"0":{"c":[0],"ch":[],"p":[2]},"1":{"c":[1],"ch":[],"p":[2]},"2":{"c":[0,1],"ch":[0,1],"p":[]}})"_json;

  if (pp.size() > 0)
    TEST_ASSERT_EQUALS("Full Map Test", true, pp.size() == 1 && json::diff(exactR, (*pp.begin())->toJson()).size() == 0)
}
