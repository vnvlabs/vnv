#include <plugins/engines/adios/AdiosEngineImpl.h>
#include <plugins/engines/adios/AdiosEngineManager.h>

#include <iostream>
#include <queue>
#include <utility>

#include "adios2.h"
#include "base/exceptions.h"
#include "c-interfaces/PackageName.h"
#include "interfaces/nodes/NodesImpl.h"
#include "json-schema.hpp"

using nlohmann::json;

using namespace VnV::Nodes;

namespace VnV {
namespace VNVPACKAGENAME {
namespace Engines {
namespace Adios {

// Put all the parser functions in @non namespace. We don't need
// them outside this unit.
namespace {

enum class AdiosReadMode { FILE };

class astream {
  bool worldStream = false;
  long id = -1;
  adios2::fstep step;
  adios2::fstream stream;

 public:
  long long commId;

  virtual ~astream() { stream.close(); }

  astream(long long cId, const std::string& filename, AdiosReadMode mode) {
    commId = cId;

    switch (mode) {
      case AdiosReadMode::FILE: {
        stream = adios2::fstream(filename, adios2::fstream::in, MPI_COMM_SELF);
      }
    }

    // initialize the thing.
    if (smallstep()) {
      commId = getVariable<long long>("commId");
      worldStream = 1 == getVariable<int>("bool");
      if (!worldStream) {
        wranks = getVector<int>("worldRank");
      }
    } else {
      throw VnVExceptionBase("Could not configure stream");
    }

  };

  long nextId() const { return id; }

  bool smallstep() { return adios2::getstep(stream, step); }

  // TODO
  long bigstep() {
    if (!adios2::getstep(stream, step)) {
      id = -1;  // Waiting
    } else {
      id = step.read<long>("identity").front();
    }
    return id;
  }

  template <typename T> std::vector<T> getVector(std::string name) {
    return step.read<T>(name);
  }

  template <typename T> T getVariable(std::string name) {
    std::vector<T> val = getVector<T>(name);
    if (!val.empty()) {
      return val.front();
    }
    throw VnVExceptionBase("Variable %s does not exist in this step",
                           name.c_str());
  }

  std::string getName() { return getVariable<std::string>("name"); }

  AdiosDataType getType() {
    return static_cast<AdiosDataType>(getVariable<int>("type"));
  }

  std::string getPackage() { return getVariable<std::string>("package"); }

  nlohmann::json getMetaData() {
    std::string mdata = getVariable<std::string>("metaData");
    return nlohmann::json::parse(mdata);
  }

  template <typename T> void addMetaData(std::shared_ptr<T> node) {
    for (const auto& it : getMetaData().items()) {
      node->getMetaData().add(it.key(), it.value());
    }
  }
  std::vector<int> wranks;
};

class IndexIter {
 public:
  std::size_t currInd = -1;
  std::vector<std::size_t> local;
  const std::vector<std::size_t>& shape;
  const std::vector<std::size_t>& offset;
  const std::vector<std::size_t>& size;
  IndexIter(const std::vector<std::size_t>& s,
            const std::vector<std::size_t>& off,
            const std::vector<std::size_t>& sze)
      : shape(s), offset(off), size(sze) {
    local.resize(off.size(), 0);
    set();
  }

  std::size_t get() const { return currInd; }

  std::size_t operator++(int x) {
    std::size_t ctemp = currInd;
    for (std::size_t i = 0; i < x; i++) {
      up();
    }
    return ctemp;
  }

  std::size_t operator++() {
    up();
    return currInd;
  }

  void up() {
    for (std::size_t i = size.size() - 1; i > -1; i--) {
      local[i]++;
      if (local[i] >= size.size()) {
        local[i] = 0;
      } else {
        set();
        return;
      }
    }
    currInd = -1;
  }

  void set() {
    currInd = 0;
    std::size_t currMult = 1;
    for (std::size_t i = shape.size() - 1; i > -1; i--) {
      currInd += currMult * (offset[i] + local[i]);
      currMult *= shape[i];
    }
  }

  int count() {
    return std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>());
  }
};

class AdiosParserImpl {
 public:
  std::shared_ptr<RootNode> rootInternal;
  long& idCounter;
  json templateJson = "";

  AdiosReadMode mode = AdiosReadMode::FILE;

  std::map<long, std::queue<std::shared_ptr<astream>>> nextVal;
  std::vector<std::shared_ptr<astream>> waiting;
  std::string filename;

  explicit AdiosParserImpl(long& idStart) : idCounter(idStart) {}

  template <typename T>
  std::shared_ptr<T> mks(const std::string& name, const std::vector<std::shared_ptr<DataBase>> &parents) {
    std::shared_ptr<T> base_ptr(new T());

    base_ptr->name = name;
    base_ptr->id = idCounter++;
    for (const auto& it : parents) {
      if ( it != nullptr) {
        base_ptr->parent = {it};
      }
    }
    rootInternal->add(base_ptr);
    return base_ptr;
  }

  template <typename T>
  std::shared_ptr<T> mks(astream& stream, const std::vector<std::shared_ptr<DataBase>> &parents) {
    std::shared_ptr<T> base_ptr = mks<T>(stream.getName(), parents);
    stream.addMetaData(base_ptr);
    return base_ptr;
  }


  template <typename T, typename V>
  std::shared_ptr<DataBase> data(astream& stream, const std::vector<std::shared_ptr<DataBase>> &parents) {
    auto n = mks<T>(stream, parents);
    n->value = { stream.getVariable<V>("value")};
    n->shape = {};
    return n;
  }

  std::shared_ptr<DataBase> booldata(astream& stream, const std::vector<std::shared_ptr<DataBase>> &parents) {
    auto n = mks<BoolNode>(stream, parents);
    n->value = { stream.getVariable<int>("value") == 1};
    n->shape = {};
    return n;
  }



  template <typename T>
  void getVec(json& j, std::vector<std::size_t>& shape, std::vector<T>& vals) {
    json sh = j["shape"];
    shape.clear();
    for (const auto& it : sh.items()) {
      shape.push_back(it.value().get<std::size_t>());
    }

    vals.clear();
    for (const auto& it : j["value"].items()) {
      vals.push_back(it.value().get<T>());
    }
  }

  template <typename T, typename V>
  void setVec(const std::string& name, json& j, std::shared_ptr<IMapNode> mapNode) {
    auto vn = mks<T>(j, {mapNode});
    getVec<V>(j, vn->shape, vn->value);
    mapNode->add(name, vn);
  }

  std::shared_ptr<DataNode> dataType(astream& stream, const std::vector<std::shared_ptr<DataBase>> &parents) {
    auto n = mks<DataNode>(stream, parents);
    n->children = mks<MapNode>("children", parents);
    n->key = stream.getVariable<long long>("dtype");

    while (stream.smallstep()) {
      if (stream.getType() != AdiosDataType::DATA_TYPE_END) {
        n->children->add(stream.getName(), data_dispatch(stream, n));
      }
    }
    return n;
  }

  // Write the vector nodes.
  std::shared_ptr<DataBase> vec(astream& stream, const std::vector<std::shared_ptr<DataBase>> &parent) {

    auto n = mks<ShapeNode>(stream, parent);

    auto dim = stream.getVariable<std::size_t>("dim");
    std::vector<std::size_t> shape = stream.getVector<std::size_t>("shape");
    std::vector<std::size_t> offsets = stream.getVector<std::size_t>("offsets");
    std::vector<std::size_t> sizes = stream.getVector<std::size_t>("sizes");

    int count =
        std::accumulate(shape.begin(), shape.end(), 1, std::multiplies<>());

    auto dtype = stream.getVariable<long long>("dtype");

    n->shape = shape;
    n->value.resize(count);
    int r = 0;
    // Loop over all the ranks
    for (std::size_t i = 0; i < offsets.size(); i += dim) {
      std::vector<std::size_t> loffset(offsets.begin() + i,
                                       offsets.begin() + i + 3);
      std::vector<std::size_t> soffset(sizes.begin() + i,
                                       sizes.begin() + i + 3);

      // TODO, we need to read this differently to get the local chunks on each
      std::vector<char> jvec = stream.getVector<char>("globalVec");
      std::string jstr(jvec.begin(), jvec.end());
      json jdata = json::parse(jstr);

      IndexIter iter(shape, loffset, soffset);
      for (auto& it : jdata.items()) {
        // Make a new data node and put it in its rightfull place.
        auto dn = mks<DataNode>("props", {n});
        auto dm = mks<MapNode>("map", {dn});
        dn->children = dm;
        dn->key = dtype;
        dn->local = true;
        n->value[iter++] = dn;

        json& dvalue = it.value();
        for (const auto& itt : dvalue.items()) {
          auto s =
              static_cast<Communication::SupportedDataType>(
                  itt.value()["type"].get<int>());
          switch (s) {
          case Communication::SupportedDataType::DOUBLE:
            setVec<DoubleNode, double>(itt.key(), itt.value(), dm);
            break;
          case Communication::SupportedDataType::LONG:
            setVec<LongNode, long>(itt.key(), itt.value(), dm);
            break;
          case Communication::SupportedDataType::STRING:
            setVec<StringNode, std::string>(itt.key(), itt.value(), dm);
            break;
          case Communication::SupportedDataType::JSON:
            setVec<JsonNode, std::string>(itt.key(), itt.value(), dm);
            break;
          }
        }
      }
    }
    return n;
  }

  void info(astream& stream) {
    auto n = mks<InfoNode>(stream, {rootInternal});
    std::string s = stream.getVariable<std::string>("json");
    json jj = json::parse(s);

    n->title = jj["title"].get<std::string>();
    n->date = jj["date"].get<long>();
    templateJson = jj["spec"];
    std::dynamic_pointer_cast<CommInfoNode>(rootInternal->commInfo)->worldSize =
        jj["worldsize"].get<int>();
    rootInternal->infoNode = n;
    rootInternal->intro = getTemplate("", "", "Introduction");
    rootInternal->concl = getTemplate("", "", "Conclusion");
  }

  std::shared_ptr<TestNode> test(astream& stream, const std::vector<std::shared_ptr<DataBase>>& parent) {
    auto n = mks<TestNode>(stream, parent);
    n->package = stream.getVariable<std::string>("package");
    int internal = stream.getVariable<int>("internal");
    n->templ = (internal > 0) ? "" : getTemplate(n->package, n->name, "Test");
    n->children = mks<ArrayNode>("children", {n});
    n->data = mks<ArrayNode>("data", {n});
    test_iter(n, stream);
    return n;
  }

  // Add data and stuff to test
  void test_iter(std::shared_ptr<TestNode>& test, astream& stream) {
    while (stream.smallstep()) {
      auto d = stream.getType();
      switch (d) {
      case AdiosDataType::TEST_END:
        return;
      case AdiosDataType::LOG:
        log(stream, std::dynamic_pointer_cast<DataBaseWithChildren>(test));
        break;
      default:
        test->data->add(data_dispatch(stream, {test->data}));
      }
    }
  }

  std::shared_ptr<UnitTestNode> addUnitTest(astream& stream) {
    if (stream.getType() == AdiosDataType::UNIT_TEST_START) {
      auto n = mks<UnitTestNode>(stream, {rootInternal->unitTests});
      n->package = stream.getPackage();
      n->templ = getTemplate(n->package, n->name, "UnitTest");
      json& testTemplates =
          templateJson["UnitTests"][n->package + ":" + n->name]["tests"];
      for (const auto& it : testTemplates.items()) {
        n->testTemplate[it.key()] = it.value().get<std::string>();
      }
      n->children = mks<ArrayNode>("results", {n});

      while (stream.smallstep()) {
        auto d = stream.getType();
        if (d == AdiosDataType::UNIT_TEST_END) {
          std::vector<std::string> nn = stream.getVector<std::string>("name");
          std::vector<std::string> m = stream.getVector<std::string>("message");
          std::vector<int> r = stream.getVector<int>("result");

          n->resultsMap = mks<MapNode>("results", {n});
          for (int i = 0; i < nn.size(); i++) {
            auto b = mks<BoolNode>("result", {n->resultsMap});
            b->value = {r[i] == 1};
            b->shape = {};
            n->resultsMap->add(nn[i], b);
          }
        } else if (d == AdiosDataType::LOG) {
          log(stream, std::dynamic_pointer_cast<DataBaseWithChildren>(n));
        } else {
          // Unit Tests can have data I guess?
          n->children->add(data_dispatch(stream, {n->children}));
        }
      }
      rootInternal->unitTests->add(n);
      return n;
    }
    throw VnVExceptionBase("Non unit test passed to unit test reader");
  }

  std::shared_ptr<DataBase> data_dispatch(astream& stream, const std::shared_ptr<DataBase>& pNode) {
    auto d = stream.getType();
    switch (d) {
    case AdiosDataType::JSON:
      return data<JsonNode, std::string>(stream, {pNode});
    case AdiosDataType::STRING:
      return data<StringNode, std::string>(stream, {pNode});
    case AdiosDataType::DOUBLE:
      return data<DoubleNode, double>(stream, {pNode});
    case AdiosDataType::BOOL:
      return booldata(stream, {pNode});
    case AdiosDataType::LONGLONG:
      return data<LongNode, long>(stream, {pNode});
    case AdiosDataType::DATA_TYPE_START:
      return dataType(stream, {pNode});
    case AdiosDataType::VECTOR_START:
      return vec(stream, {pNode});
    default:
      throw VnVExceptionBase(
          "Invalid -- We should never catch these in a dispatcher. ");
    }
  }

  // I am going with two maps to make it faster to find nodes at an iteration
  // Leaf nodes are sorted by id,
  // openPoints are sorted by comm. Since the comm is constant we can always find
  // it by pulling the top of the stack for that comm.
  std::map<long,std::shared_ptr<DataBase>> leafNodes;
  std::map<long long, std::stack<std::shared_ptr<InjectionPointNode>>> openPoints;

  std::shared_ptr<InjectionPointNode>& getOpenInjectionPointNode(
      astream& stream) {

    auto it = openPoints.find(stream.commId);
    if (it == openPoints.end() || it->second.empty()) {
      throw VnVExceptionBase("Nesting error");
    } else {
      return it->second.top();
    }
  }

  void openInjectionPoint(astream& stream,
                          std::shared_ptr<InjectionPointNode>& node) {

    // Parents are no longer leaf nodes.
    for (const auto& it : node->parent) {
        leafNodes.erase(it->getId());
    }
    leafNodes[node->id] = node;

    auto it = openPoints.find(stream.commId);
    if (it == openPoints.end()) {
       openPoints.emplace(stream.commId, std::move(std::stack<std::shared_ptr<InjectionPointNode>>({node})));
    } else {
      it->second.push(node);
    }

    node->open = true;
  }

  void closeInjectionPoint(astream& stream,
                           std::shared_ptr<InjectionPointNode>& node) {

    //Remove node from the leaf nodes map.
    leafNodes.erase(node->getId());
    node->open = false;
    openPoints[stream.commId].pop();
    // Now we need to add back parent leaf nodes.
    // A node is a leaf node if it is open and has no open children.
    // If it is not open, then its pass to parent.
    std::set<int> added;
    bool childAdded = false;
    for (const auto& it : node->parent) {
      resetLeafNodes(it);
    }

  }

  bool resetLeafNodes(const std::shared_ptr<DataBase> &node) {

    if (node->open) {
      switch (node->getType()) {
       case DataBase::DataType::Root:
       case DataBase::DataType::InjectionPoint: {
        auto a = std::dynamic_pointer_cast<DataBaseWithChildren>(node);
        auto children = (ArrayNode*)a.get();
        bool ccadded = false;
        for (int i = 0; i < children->size(); i++) {
          if ( resetLeafNodes(children->getShared(i)) ) {
            ccadded = true;
          }
        }
        if (!ccadded) {
          leafNodes[node->getId()] = node;
        }
        return true;
      }
      default:
        break;
      }
    }
    return false;
  }


  void setAdded(const std::shared_ptr<DataBase>& node, std::set<long>&added) {
    added.insert(node->id);
    for (auto &it : node->parent) {
       setAdded(it, added);
    }
  }

    void setParents(const std::shared_ptr<RootNode>& leafNode,  std::shared_ptr<DataBase> &node, long nodeComm, std::set<long> &added) {
      if (added.find(leafNode->getId()) != added.end()) {
          return; // Already sorted.
      }
      node->parent.push_back(leafNode);
      leafNode->getChildren()->add(node);
      setAdded(leafNode, added);
    }

    void setParents(const std::shared_ptr<InjectionPointNode>& leafNode,  std::shared_ptr<DataBase> &node, long long nodeComm, std::set<long> &added) {
      if (added.find(leafNode->getId()) != added.end()) {
        return; // Already sorted.
      } else if (dynamicCommMap->searchNodeInMainNodeCommChain(leafNode->commId, nodeComm) ) {
          node->parent.push_back(leafNode);
          leafNode->getChildren()->add(node);
          setAdded(leafNode, added);
        } else {
          for (auto &it : leafNode->parent) {
            if (it->getType() == DataBase::DataType::InjectionPoint) {
              setParents(std::dynamic_pointer_cast<InjectionPointNode>(it),node,nodeComm,added);
            } else if (it->getType() == DataBase::DataType::Root) {
              setParents(std::dynamic_pointer_cast<RootNode>(it), node,
                         nodeComm, added);
            } else {
              throw VnVExceptionBase("Parent that is not an injection point or root node.  ");
            }
          }
        }
  }

  void setParents( astream& stream, std::shared_ptr<DataBase> node ) {
    std::set<long> added;
    for (auto &it : leafNodes) {
      switch(it.second->getType()) {

      case DataBase::DataType::Root: {
        setParents(std::dynamic_pointer_cast<RootNode>(it.second), node, stream.commId, added);
      }

      case DataBase::DataType::InjectionPoint: {
        setParents(std::dynamic_pointer_cast<InjectionPointNode>(it.second), node, stream.commId, added);
      }

      default:
        break;

      }
    }
  }

  std::shared_ptr<DynamicCommMap> dynamicCommMap = nullptr;
  void updateCommMap(std::vector<std::shared_ptr<astream>> &newstreams) {
    for (const auto& it : newstreams) {
      dynamicCommMap->append(it->commId, it->wranks);
    }
    json& jcomm = std::dynamic_pointer_cast<CommInfoNode>(rootInternal->commInfo)->commMap;

    std::set<long> done1;
    dynamicCommMap->getRootCommunicator()->toJson1(jcomm,done1);
  }

  std::shared_ptr<RootNode> read_from_file(const std::string& directory) {
    this->mode = AdiosReadMode::FILE;
    this->filename = directory;

    initRootNode();

    std::string metadatafile = AdiosEngineManager::getMetaDataFileName(directory);
    std::ifstream ifs(metadatafile);
    json j = json::parse(ifs);
    std::map<long long, std::string> fileComms = {};

    json& cmp = j["comms"];
    for (const auto& it : cmp.items()) {
      fileComms[it.value().get<long long>()] = it.key();
    }

    std::dynamic_pointer_cast<CommInfoNode>(rootInternal->commInfo)->worldSize = j["worldsize"].get<int>();
    dynamicCommMap.reset(new DynamicCommMap(rootInternal->commInfo->getWorldSize()));

    loadComms(fileComms);
    run();
    return rootInternal;
  }

  std::map<long long, std::string> fetchCommunicators() const {
    if (mode == AdiosReadMode::FILE) {
      return {};
    }
    return {};
  }

  std::shared_ptr<LogNode> log(astream& stream, const std::shared_ptr<DataBaseWithChildren>& p ) {
    if (stream.getType() == AdiosDataType::LOG) {

      auto n = mks<LogNode>(stream, {});

      n->package = stream.getPackage();
      n->level = stream.getVariable<std::string>("levelVal");
      n->message = stream.getVariable<std::string>("message");
      n->stage = std::to_string(stream.getVariable<int>("stageVal"));
      n->comm = std::to_string(stream.commId);

      if (p == nullptr) {
        setParents(stream, n);
      } else {
        n->parent.push_back(std::dynamic_pointer_cast<DataBase>(p));
      }
      return n;
    }
    throw VnVExceptionBase("Invalid Step passed to gen Log Node");
  }

  std::shared_ptr<InjectionPointNode> injectionPoint(
      astream& stream, const std::shared_ptr<DataBaseWithChildren>& p) {

    auto n = mks<InjectionPointNode>(stream, {});
    n->package = stream.getPackage();
    n->templ = getTemplate(n->package, n->name, "InjectionPoint");
    n->commId = stream.commId;
    n->children = mks<ArrayNode>("children", {n});
    n->tests = mks<ArrayNode>("tests", {n});
    n->startIndex = stream.getVariable<long>("identity");

    setParents(stream,n);

    while (stream.smallstep()) {
      AdiosDataType d = stream.getType();
      switch (d) {
      case AdiosDataType::INJECTION_POINT_SINGLE_END: {
        n->endIndex = stream.getVariable<long>("identity");
        return n;
      }
      case AdiosDataType::INJECTION_POINT_BEGIN_END: {
        openInjectionPoint(stream, n);
        return n;
      }
      case AdiosDataType::TEST_START: {
        auto t = test(stream, {n->tests});
        n->tests->add(t);
        break;
      }
      case AdiosDataType::LOG: {
        log(stream, std::dynamic_pointer_cast<DataBaseWithChildren>(n));
        break;
      }
      default:
        throw VnVExceptionBase(
            "Unexpected data type within INJECTION_BEGIN step.");
      }
    }
    throw VnVExceptionBase("Ran out of steps without finding an end. ");
  }

  std::shared_ptr<InjectionPointNode> injectionPoint_iter(astream& stream) {
    // TODO Find the injection point that is open.
    std::shared_ptr<InjectionPointNode> n = getOpenInjectionPointNode(stream);

    // TODO We assume tests come through in the same order every time. Is this
    // TODO gauranteed? Could change test to a MapNode and search on package
    // name and test name but that would add the restriction of not putting the
    // same test on an injection poitn more than one time (which might not be a
    // bad thing).
    int testCounter = 0;

    while (stream.smallstep()) {
      auto d2 = stream.getType();
      switch (d2) {
      case AdiosDataType::TEST_START: {
        // Test was started.
        auto t = std::dynamic_pointer_cast<TestNode>(
            n->tests->getShared(testCounter++));
        test_iter(t, stream);
        break;
      }
      case AdiosDataType::INJECTION_POINT_ITER_END: {
        return n;
      }
      case AdiosDataType::INJECTION_POINT_END_END: {
        n->endIndex = stream.getVariable<long>("identity");
        closeInjectionPoint(stream, n);
        return n;
      }
      case AdiosDataType::LOG: {
        log(stream, std::dynamic_pointer_cast<DataBaseWithChildren>(n));
        break;
      }
      default:
        throw VnVExceptionBase(
            "Unexpected data type inside injection point iteration. ");
      }
    }
    return n;
  }

  std::string getTemplate(const std::string& pname, const std::string& key,
                          const std::string& type) {
    try {
      if (type == "UnitTest") {
        return templateJson["UnitTests"][pname + ":" + key]["docs"]
            .get<std::string>();

      } else if (type == "Test") {
        return templateJson["Tests"][pname + ":" + key]["docs"]
            .get<std::string>();

      } else if (type == "InjectionPoint") {
        return templateJson["InjectionPoints"][pname + ":" + key]["docs"]
            .get<std::string>();

      } else if (type == "DataType") {
        return templateJson["DataTypes"][key]["docs"].get<std::string>();

      } else if (type == "Introduction" || type == "Conclusion") {
        return templateJson[type]["docs"].get<std::string>();

      } else {
      }
    } catch (...) {
      // It doesn't exist because of an error
    }
    return "";
  }


  std::shared_ptr<RootNode> initRootNode() {
    rootInternal.reset(new RootNode());  // For id tracking in a single loop.
    rootInternal->id = idCounter++;
    rootInternal->name = "Root";
    rootInternal->parent = {};
    rootInternal->children = mks<ArrayNode>("children", {rootInternal});
    rootInternal->unitTests = mks<ArrayNode>("unitTests", {rootInternal});
    rootInternal->commInfo = mks<CommInfoNode>("commMap", {rootInternal});
    return rootInternal;
  }

  long addToNextVal(const std::shared_ptr<astream>& stream, bool addToWaiting) {
    long nextId = stream->nextId();

    if (nextId > -1) {
      auto fnex = nextVal.find(nextId);
      if (fnex == nextVal.end()) {
        std::queue<std::shared_ptr<astream>> q;
        q.push(stream);
        nextVal[nextId] = std::move(q);
      } else {
        fnex->second.push(stream);
      }
    } else if (addToWaiting && nextId == -1) {
      waiting.push_back(stream);
    }
    return nextId;
  }

  void run() {
    while (!nextVal.empty() && !waiting.empty()) {
      loop_body();
    }
  }

  void loadComms(const std::map<long long, std::string>& newcomms) {
    if (!newcomms.empty()) {
      std::vector<std::shared_ptr<astream>> s;
      s.reserve(newcomms.size());
      for (auto& it : newcomms) {
        auto nc = std::make_shared<astream>(it.first,it.second,mode);
        addToNextVal(nc, true);
        s.push_back(nc);
      }
      updateCommMap(s);
    }
  }

  void loop_body() {
    // Step 1: Load all new communicators;
    loadComms(fetchCommunicators());

    // Step 2: See if anyone revieved anything.
    refreshWaiting();

    // Step 3. Get the next value.
    if (!nextVal.empty()) {
      auto it = nextVal.begin();
      auto itt = it->second.front();

      // Take the step.
      step(*itt);

      // Add the next val to the thing a mi jig
      addToNextVal(itt, true);

      // Pop the stream from the queue, pop the queue from the list (if empty).
      it->second.pop();
      if (it->second.empty()) {
        nextVal.erase(it);
      }
    }
  }

  // This is the next step to process. We must incorperate it.
  void step(astream& stream) {
    // We need to dispatch this.
    auto d = stream.getType();
    switch (d) {
    case AdiosDataType::INJECTION_POINT_BEGIN_BEGIN:
    case AdiosDataType::INJECTION_POINT_SINGLE_BEGIN: {
      injectionPoint(stream, nullptr);
      break;
    }
    case AdiosDataType::INJECTION_POINT_ITER_BEGIN:
    case AdiosDataType::INJECTION_POINT_END_BEGIN: {
      injectionPoint_iter(stream);
      break;
    }
    case AdiosDataType::UNIT_TEST_START: {
      addUnitTest(stream);
      break;
    }
    case AdiosDataType::INFO: {
      info(stream);
    }
    case AdiosDataType::LOG: {
      log(stream, nullptr);
      break;
    }
    default: {
      throw VnVExceptionBase("Unknown or unexpected type");
    }
    }
    // Big step updates the ids.
    stream.bigstep();
  }

  void refreshWaiting() {
    for (auto i = waiting.begin(), last = waiting.end(); i != last;) {
      // Get the next Id, adding it to the map if one is available.
      long nextId = addToNextVal(*i, false);

      // If next id is not longer -1, remove it.
      // -2 indicates the thing is Done.
      if (nextId >= 0 || nextId == -2) {
        waiting.erase(i);
      } else {
        ++i;
      }
    }
  }
};

}  // namespace


}  // namespace Adios

std::shared_ptr<Nodes::IRootNode> AdiosEngineManager::readFromFile(std::string filename, long& counter) {
   Adios::AdiosParserImpl handler(counter);
   return handler.read_from_file(filename);
}

}  // namespace Engines
}  // namespace VNVPACKAGENAME
}  // namespace VnV
