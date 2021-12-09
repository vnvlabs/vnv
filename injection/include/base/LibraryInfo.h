#ifndef LIBRARY_INFO_HEADER_H
#define LIBRARY_INFO_HEADER_H

#include <string>

#include "json-schema.hpp"

using nlohmann::json;

namespace VnV {
namespace DistUtils {

struct libInfo {
  std::string name = "<unknown>";
  long timestamp = 0;
  long size = 0;

  json toJson() const {
    json j = json::object();
    j["name"] = name;
    j["timestamp"] = timestamp;
    j["size"] = size;
    return j;
  }

  void fromJson(const json& j) {
    name = j["name"].get<std::string>();
    timestamp = j["timestamp"].get<long>();
    size = j["size"].get<long>();
  }
};

struct libData {
  std::vector<libInfo> libs;
  libData() {}
};

}  // namespace DistUtils
}  // namespace VnV

#endif
