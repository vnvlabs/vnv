
#include <algorithm>
#include <list>
#include <map>
#include <memory>
#include <set>

#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {



class CommMap : public ICommMap {
  
  class Comm;
  typedef std::shared_ptr<Comm> Comm_ptr;

  class Comm {
    void getAllChildren(std::set<long>& data,
                        std::map<long, Comm_ptr>& result) {
      for (auto& it : children) {
        if (data.find(it.first) == data.end()) {
          it.second->getAllChildren(data, result);
          data.insert(it.first);
          result.erase(it.first);
        }
      }
    }

   public:
    long id;
    std::set<int> procs;
    std::map<long, Comm_ptr> children;
    std::map<long, Comm_ptr> parents;

    Comm(std::set<int> procs_, long id_) : procs(procs_), id(id_) {}

    bool isRoot() { return parents.size() == 0; }

    std::map<long, Comm_ptr>& getChildren() { return children; }

    void getAllChildren(std::map<long, Comm_ptr>& data) {
      for (auto& it : children) {
        if (data.find(it.first) == data.end()) {
          it.second->getAllChildren(data);
          data.insert(it);
        }
      }
    }

    void getDirectChildren(bool strip, std::map<long, Comm_ptr>& data) {
      if (!strip) {
        getAllChildren(data);
      } else {
        // A direct child is any child that is not also a child of one of  my
        // children.
        std::set<long> grandChildren;
        for (auto& it : children) {
          // If we havnt found this as a grand child already
          if (grandChildren.find(it.first) == grandChildren.end()) {
            // add it to my list of all direct children.
            data.insert(it);
            // Now add all its children -- it will be removed if found as a
            // grand child.
            it.second->getAllChildren(grandChildren, data);
          }
        }
      }
    }

    std::map<long, Comm_ptr>& getParents() { return parents; }

    void getAllParents(std::map<long, Comm_ptr>& data) {
      for (auto& it : parents) {
        if (data.find(it.first) == data.end()) {
          data.insert(it);
          it.second->getAllParents(data);
        }
      }
    }

    nlohmann::json getParentChain() {
      nlohmann::json j = nlohmann::json::array();
      std::map<long, Comm_ptr> m;
      getAllParents(m);
      for (auto it : m) {
        j.push_back(it.first);
      }
      return j;
    }
    nlohmann::json getChildChain() {
      nlohmann::json j = nlohmann::json::array();
      std::map<long, Comm_ptr> m;
      getAllChildren(m);
      for (auto it : m) {
        j.push_back(it.first);
      }
      return j;
    }

    void toJson(bool strip, nlohmann::json& j, std::set<long>& done) {
      if (done.find(id) == done.end()) {
        done.insert(id);
        nlohmann::json node = nlohmann::json::object();
        node["id"] = id;
        node["group"] = procs.size();
        node["parents"] = getParentChain();
        node["children"] = getChildChain();
        j["nodes"].push_back(node);

        std::map<long, Comm_ptr> ch;
        getDirectChildren(strip, ch);

        for (auto& it : ch) {
          nlohmann::json link = nlohmann::json::object();
          link["target"] = it.first;
          link["source"] = id;
          link["value"] = it.second->procs.size();
          j["links"].push_back(link);

          it.second->toJson(strip, j, done);
        }
      }
    }
  };

  std::map<long, Comm_ptr> nodes;

  bool addChild(Comm_ptr ptr1, Comm_ptr ptr2) {
    // A Comm is my child if I contain all of its processors.
    // That can only be true if it is smaller than me.
    int sizeDiff = ptr1->procs.size() - ptr2->procs.size();

    Comm_ptr parent, child;

    if (sizeDiff == 0) {
      return;  // Cant be child if same size. (assume unique)
    } else if (sizeDiff < 0) {
      parent = ptr2;
      child = ptr1;
    } else {
      child = ptr2;
      parent = ptr1;
    }

    auto child_it = child->procs.begin();
    auto parent_it = parent->procs.begin();

    while (child_it != child->procs.end() && parent_it != parent->procs.end()) {
      if (*parent_it == *child_it) {
        ++parent_it;
        ++child_it;  // Found this one. so move on to next for both.
      } else if (*parent_it < *child_it) {
        ++parent_it;  // parent is less -- this is ok, parent can have elements
                      // not in child.
      } else {
        return;  // parent is greater -- Not ok, this means child_it is not in
                 // parent --> not child.
      }
    }

    if (child_it == child->procs.end()) {
      child->parents.insert(std::make_pair(parent->id, parent));

      parent->children.insert(std::make_pair(child->id, child));

    } else {
      return;  // We ran out of parent elements to check so it wqasnt there.
    }
  }

 public:
  void add(long id, std::set<long>& comms) {
    Comm_ptr p = std::make_shared<Comm>(comms, id);
    for (auto& it : nodes) {
      addChild(p, it.second);
    }
    nodes.insert(std::make_pair(p->id, p));
  }

  nlohmann::json toJson(bool strip) {
    nlohmann::json j = R"({"nodes":[],links:[]})"_json;
    std::set<long> done;
    for (auto& it : nodes) {
      it.second->toJson(strip, j, done);
    }
    return j;
  }



};
}