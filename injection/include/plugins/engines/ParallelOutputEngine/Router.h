/*
 * router.h
 *
 *  Created on: Mar 4, 2020
 *      Author: gw6
 */

#ifndef VV_PARALLELOUTPUTENGINE_ROUTER_H_
#define VV_PARALLELOUTPUTENGINE_ROUTER_H_

//#ifdef WITH_MPI
//#include <mpi.h>
//#endif /* WITH_MPI */

#include <thread>
#include <vector>

#include "SparseBitVector.h"
#include "interfaces/ICommunicator.h"

typedef SparseBitVector Route;

class Router {
 private:
  int m_fanout = 5;
  int m_size = 1;
  int m_id = 0;
  int m_parent = 0;
  int m_root = 0;
  Route m_children;
  std::unordered_map<std::string, std::unordered_map<std::string, Route>>
      put_map;

  VnV::ICommunicator_ptr m_comm;

  //#ifdef WITH_MPI
  //    MPI_Comm m_comm;
  //#endif /* WITH_MPI */
  int parent_of(int id, int root, int fanout);
  Route children_of(int id, int root, int fanout);
  void init(VnV::ICommunicator_ptr ptr);
  int send(int id, Route route, const std::string name,
           const std::string value);

 public:
  Router(VnV::ICommunicator_ptr ptr);
  Router(VnV::ICommunicator_ptr ptr, int fanout);
  Router(int id, int root, int size, int fanout);  // Does not set comm ?
  ~Router() {}
  int send(std::string name, std::string value);
  int forward();
  bool isRoot() { return m_id == m_root; }
  int parent_of() { return m_parent; }
  Route children_of() { return m_children; }
};

#endif /* INJECTION_SRC_PLUGINS_ENGINES_PARALLELOUTPUTENGINE_ROUTER_H_ */
