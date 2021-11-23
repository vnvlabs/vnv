
#ifndef STREAMREADER_TEMPLATES_H
#define STREAMREADER_TEMPLATES_H

#include <thread>
#include <mutex>

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
  std::mutex mu;
  std::queue<JsonElement> content;

  void getLine(json& current, long& currentValue) override {
    std::lock_guard<std::mutex> lock(mu);
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

  bool hasNext() override { 
    std::lock_guard<std::mutex> lock(mu);
    return content.size() > 0;
  }

  long streamId() const override { return sId; }

  void add(long i, const json& d) {
    std::lock_guard<std::mutex> lock(mu);
    JsonElement j(i, d);
    content.emplace(j);
  }

  ~JsonSingleStreamIterator() {}
};

class JsonPortStreamIterator : public MultiStreamIterator<VnV::StreamReader::JsonSingleStreamIterator, json> {
protected:  
  int port = 0;
  struct MHD_Daemon* daemon = NULL;
  std::mutex mu; 
  bool doneMessageRecieved = false;
  std::map<std::string,json> responses;

 public:
  JsonPortStreamIterator(std::string p) : MultiStreamIterator<VnV::StreamReader::JsonSingleStreamIterator, json>() {
      port = std::atoi(p.c_str());
  };

  void done() { doneMessageRecieved = true; }

  virtual bool isDone() override {
    if (!doneMessageRecieved) {
      return false;
    }
    return !hasNext();
  }

  virtual void respond(long id, long jid, const json& response) override {
    
    //Wait for it to finish reading. 
    std::lock_guard<std::mutex> lock(mu);
    std::ostringstream oss;
    oss << id << ":" << jid;
    responses[oss.str()] = json::parse(response.dump()); //Deep copy.]
  }
  
  virtual bool getResponse(long id, long jid, json&response) {
    
    std::lock_guard<std::mutex> lock(mu);
    std::ostringstream oss;
    oss << id << ":" << jid;
    std::string key = oss.str();
    auto it = responses.find(key);
    if (it != responses.end()) {
      response = json::parse(it->second.dump());  
      responses.erase(it);
      return true;
    }
    
    return false;
  }

  ~JsonPortStreamIterator() { }
};


class JsonStreamVisitor : public ParserVisitor<json> {
public:  
  virtual void setWriteLock() override {}
  virtual void releaseWriteLock() override {}

};


}  // namespace StreamReader
}  // namespace VnV

#endif