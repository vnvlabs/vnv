
#ifndef STREAMREADER_TEMPLATES_H
#define STREAMREADER_TEMPLATES_H

#include <thread>

#include "plugins/engines/streaming/streamtemplate.h"

using namespace VnV::VNVPACKAGENAME::Engines::Streaming;

namespace VnV {

namespace StreamReader {

class JsonElement {
 public:
  long id;
  json data;
  JsonElement(long i, const json& d) : id(i), data(d) {}
};

class JsonSingleStream {
 public:
  std::queue<JsonElement> content;
  std::stack<std::shared_ptr<DataBase>> s;

  JsonElement next() {
    auto j = content.front();
    content.pop();
    return j;
  }

  const JsonElement& get() const { return content.front(); }

  void add(long id, const json& data) {
    content.emplace(JsonElement(id, data));
  }

  void push(std::shared_ptr<DataBase> d) { s.push(d); }
  std::shared_ptr<DataBase> pop() {
    if (s.size() > 0) {
      auto ss = s.top();
      s.pop();
      return ss;
    }
    return nullptr;
  }
  std::shared_ptr<DataBase> top() {
    if (s.size() > 0) {
      return s.top();
    }
    return nullptr;
  }
};

class JsonStreamIterator : public Iterator<json> {
 protected:
  typedef std::pair<long, JsonSingleStream> SEntry;

  std::map<long, JsonSingleStream> instreams;
  typename std::map<long, JsonSingleStream>::iterator min;

  virtual void getLine(json& current, long& cid) override {
    min = std::min_element(instreams.begin(), instreams.end(),
                           [](const SEntry& x, const SEntry& y) {
                             return (x.second.get().id < y.second.get().id);
                           });
  }

 public:
  JsonStreamIterator() : Iterator<json>(){};

  virtual bool start_stream_reader() = 0;
  virtual void stop_stream_reader() = 0;
  virtual bool stream_reader_running() const = 0;

  virtual ~JsonStreamIterator() {}

  virtual bool isDone() const { return stream_reader_running(); }

  virtual bool hasNext() const {
    // Has next if we have another one.
    for (auto it : instreams) {
      if (it.second.content.size() > 0) {
        return true;
      }
    }
    return false;
  }

  virtual long streamId() const override { return (*min).first; }

  virtual std::pair<json, long> next() override {
    this->pullLine(false);
    JsonElement e = (*min).second.get();
    return {e.data, e.id};
  }

  virtual long peekId() {
    this->pullLine(true);
    return (*min).second.get().id;
  }

  void push(std::shared_ptr<DataBase> d) override { (*min).second.push(d); }

  std::shared_ptr<DataBase> pop() override { return (*min).second.pop(); }

  std::shared_ptr<DataBase> top() override { return (*min).second.top(); }
};

// Wrap the root node with a parser and thread so we dont
// loose these until the node is deleted.
class RootNodeWithThread : public RootNode {
 public:
  std::shared_ptr<ParserVisitor<json>> visitor;
  std::shared_ptr<JsonStreamIterator> stream;
  std::thread worker;

  void run() {
    stream->start_stream_reader();
    worker = std::thread(&ParserVisitor<json>::process, visitor.get());
  }

  static std::shared_ptr<IRootNode> parse(
      long& id, std::shared_ptr<JsonStreamIterator> stream) {
    std::shared_ptr<RootNodeWithThread> root =
        std::make_shared<RootNodeWithThread>();
    root->stream = stream;
    root->id = id++;
    root->name = "ROOT";
    root->visitor = std::make_shared<ParserVisitor<json>>(stream, id, root);
    root->run();
    return root;
  }
};

}  // namespace StreamReader
}  // namespace VnV

#endif