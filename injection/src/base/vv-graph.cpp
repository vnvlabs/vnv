#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>

namespace VnV {
namespace  Graph {

class Graph
{

   // Nodes are the classes, edges are the trasform names.
   std::map<std::size_t,std::string> reverseNodeMap;
   std::map<std::string, std::size_t> nodeMap;
   std::map<std::size_t, std::set<std::pair<std::size_t,std::string>>> adj;
   public:

        Graph(); // Constructor
        std::size_t addNode(std::string n);
        void addEdge(std::string v, std::string w, std::string edgeType); // function to add an edge to graph
        std::list<std::string> getTransformList(std::string from, std::string to);

private:
        std::list<std::size_t> getPath(std::size_t s, std::size_t d); // returns true if there is a path from s to d

};

Graph::Graph()  {
}

std::list<std::string> Graph::getTransformList(std::string from, std::string to) {
    if ( from.compare(to) == 0 ) return {"default"};

    // This represents the BFS for the graph. Basically, we need to iterate through this, to
    // find a path to the node without false results.
    std::list<std::size_t> p = getPath(nodeMap[from],nodeMap[to]);
    std::list<std::string> ret;

    auto it = p.begin();
    while (it != p.end()) {
        auto node = adj[*it]; // get the node this refers to.
        ret.push_back()
    }

    for (auto it : p ) {
        ret.push_back("");
    }
    return ret;

}

std::size_t Graph::addNode(std::string n) {
    auto src = nodeMap.insert(std::make_pair(n,nodeMap.size()));
    if ( src.second ) {
        reverseNodeMap.insert(std::make_pair(src.first->second,n));
        adj.insert(std::make_pair(src.first->second, std::set<std::pair<std::size_t,std::string>>()));
    }
    return src.first->second;
}

void Graph::addEdge(std::string v, std::string w, std::string edgeType) {
    std::size_t src = addNode(v);
    adj[src].insert(std::make_pair(addNode(w),edgeType)); // Add w to v’s list.
}


// A BFS based function to check whether d is reachable from s.
std::list<std::size_t> Graph::getPath(std::size_t s, std::size_t d) {
   if (s == d) {
        return {};

    };

    std::vector<bool> visited(nodeMap.size());
    std::list<std::size_t> queue = {s};

    // Mark the current node as visited and enqueue it
    visited[s] = true;
    std::list<std::size_t> path;

    while (!queue.empty())
    {
        // Dequeue a vertex from queue and print it
        s = queue.front();
        path.push_back(s);
        queue.pop_front();

        // Get all adjacent vertices of the dequeued vertex s
        // If a adjacent has not been visited, then mark it visited
        // and enqueue it
        for (auto i = adj[s].begin(); i != adj[s].end(); ++i)
        {
            // If this adjacent node is the destination node, then return true
            if (i->first == d) {
                path.push_back(d);
                return path;
            }

            // Else, continue to do BFS
            if (! visited[i->first]) {
                visited[i->first] = true;
                queue.push_back(i->first);
            }
        }
    }
    return {};
}
}
}
