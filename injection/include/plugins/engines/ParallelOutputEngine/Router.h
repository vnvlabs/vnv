/*
 * router.h
 *
 *  Created on: Mar 4, 2020
 *      Author: gw6
 */

#ifndef VV_PARALLELOUTPUTENGINE_ROUTER_H_
#define VV_PARALLELOUTPUTENGINE_ROUTER_H_

#include <mpi.h>

#include <vector>
#include <thread>

#include "SparseBitVector.h"

typedef SparseBitVector Route;

class Router {
private:
    int m_fanout = 5;
    int m_size = 1;
    int m_id;
    int m_parent = 0;
    int m_root;
    Route m_children;
    bool m_running = false;
    std::unordered_map<std::string, std::unordered_map<std::string, Route>> put_map;
#if WITH_MPI
    MPI_Comm m_comm;
#endif /* WITH_MPI */
    int parent_of(int node);
    Route children_of(int node);
    void init();
    int send(int id, Route route, const std::string name, const std::string value);
    int parent_of() {return m_parent;}
    Route children_of() {return m_children;}
    void start();
    void stop();

public:
    Router();
    Router(int fanout);
    ~Router();
    int send(std::string name, std::string value);
    int forward();
    bool isRoot() { return m_id == m_root; }
};

#endif /* INJECTION_SRC_PLUGINS_ENGINES_PARALLELOUTPUTENGINE_ROUTER_H_ */
