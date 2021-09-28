
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

// A static file iterator.
class JsonSingleStreamIterator : public Iterator<json> {
  long sId;
  std::queue<JsonElement> content;

  void getLine(json& current, long& currentValue) override {
    if (content.size() > 0) {
      std::string currline;
      current = content.front().data;
      currentValue = content.front().id;
      content.pop();
    } else {
      throw VnV::VnVExceptionBase("get line called when no line available");
    }
  }

 public:
  JsonSingleStreamIterator(long streamId_) : sId(streamId_) {}

  bool hasNext() override { return content.size() > 0; }

  long streamId() const override { return sId; }

  void add(long i, const json& d) {
    JsonElement j(i, d);
    content.emplace(j);
  }

  ~JsonSingleStreamIterator() {}
};

class JsonStreamIterator
    : public MultiStreamIterator<JsonSingleStreamIterator, json> {
 public:
  virtual bool start_stream_reader() = 0;
  virtual void stop_stream_reader() = 0;
};


}  // namespace StreamReader
}  // namespace VnV

#endif