/*
 * router.cpp
 *
 */

#include "plugins/engines/ParallelOutputEngine/Router.h"
#include <msgpack.hpp>

int Router::parent_of(int node) {
    return (node - 1) / m_fanout;
}

Route Router::children_of(int node) {
    SparseBitVector children;

    for (int c = 1; c <= m_fanout; c++) {
        int child = m_fanout * node + c;
        if (child >= m_size) {
            break;
        }
        children.set(child);
    }
    return children;
}

void Router::start() {
    if (!m_running) {
        m_running = true;
    }
}

void Router::stop() {
    if (m_running) {
        m_running = false;
    }
}

// Send to my parent
int Router::send(std::string name, std::string value) {
    Route route(m_id);
    put_map[name][value] = route;
    return 0;
}

// Send to node
int Router::send(int id, Route route, std::string name, std::string value) {
#ifdef WITH_MPI
    if (m_id != m_root) {
        std::stringstream ss;
        msgpack::pack(ss, route);
        msgpack::pack(ss, name);
        msgpack::pack(ss, value);
        return MPI_Send(ss.str().data(), ss.str().length(), MPI_CHAR, id, 0, m_comm);
    }
#endif /* WITH_MPI */
    return 0;
}

// Receive any messages from children and forward to the parent
//
// We keep waiting for messages until an "end of messages" message
// is received.
//
// We always send at least one message to the parent to indicate the end
// of the messages.
int Router::forward() {
    int len;
    int id;
    int msg_avail;
    Route route;
    Route children(m_children);
    std::string buf;
    std::string name;
    std::string value;
    std::unordered_map<std::string, std::unordered_map<std::string, Route>> name_map;
    MPI_Status status;

    // Check for messages until we've received an
    // end message from them all
    while (!children.empty()) {
        for (const int child : children) {
            // Check for a message from the child
            MPI_Iprobe(child, MPI_ANY_TAG, m_comm, &msg_avail, &status);
            if (msg_avail) {
                if (status.MPI_TAG == 1) {
                    // Receive end message
                    MPI_Recv(&id, 1, MPI_INTEGER, child, 1, m_comm, &status);
                    children.reset(child);
                } else {
                    // Receive a normal message. The message is variable length, so find out how long
                    // it is from the probe status
                    MPI_Get_count(&status, MPI_CHAR, &len);
                    std::string buf;
                    buf.resize(len);
                    MPI_Recv(&buf[0], len, MPI_CHAR, child, 0, m_comm, &status);

                    // Unpack the message into: route, name, value
                    std::size_t offset = 0;
                    msgpack::object_handle oh = msgpack::unpack(buf.data(),
                            buf.size(), offset);
                    msgpack::object route_obj = oh.get();
                    route_obj.convert(route);

                    oh = msgpack::unpack(buf.data(), buf.size(), offset);
                    msgpack::object name_obj = oh.get();
                    name_obj.convert(name);

                    oh = msgpack::unpack(buf.data(), buf.size(), offset);
                    msgpack::object val_obj = oh.get();
                    val_obj.convert(value);

                    // Retrieve the current route from the map. Note that this
                    // will create an empty route if the name/value is not
                    // in the map.
                    Route& map_route = put_map[name][value];

                    // Merge the received route with the current map route
                    map_route |= route;
                }
            }
        }
    }

    // Send all distinct messages from hash to parent
    for (auto name_it = put_map.begin(); name_it != put_map.end(); name_it++) {
        for (auto value_it = name_it->second.begin(); value_it != name_it->second.end(); value_it++) {
            if (!Router::isRoot()) {
                Router::send(m_parent, value_it->second, name_it->first, value_it->first);
            } else {
                printf("PARALLEL ENGINE PUT %s: %s = %s\n", value_it->second.toString().c_str(),
                        name_it->first.c_str(), value_it->first.c_str());
            }
        }
    }

    put_map.clear();

    // Tell parent we have finished sending
    MPI_Send(&m_id, 1, MPI_INTEGER, m_parent, 1, m_comm);

    return 0;
}

void Router::init() {
#ifdef WITH_MPI
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &m_size);
    MPI_Comm_dup(MPI_COMM_WORLD, &m_comm);

    m_id = id;
    m_root = 0;

    // Find our parent
    if (id > 0) {
        m_parent = (id - 1) / m_fanout;
    }

    // Find our children
    for (int c = 1; c <= m_fanout; c++) {
        int child = m_fanout * id + c;
        if (child >= m_size) {
            break;
        }
        m_children.set(child);
    }
#endif /* WITH_MPI */
}

Router::Router() {
    init();
}

Router::Router(int fanout) {
    m_fanout = fanout;
    init();
}

Router::~Router() {
    stop();
}

