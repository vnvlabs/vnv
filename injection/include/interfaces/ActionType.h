#ifndef VNV_ACTION_TYPE_HEADER
#define VNV_ACTION_TYPE_HEADER

namespace VnV {

namespace ActionStage {

typedef std::string type;
constexpr auto init = "i";
constexpr auto start = "s";
constexpr auto iter = "it";
constexpr auto end = "e";
constexpr auto final = "f";

};  // namespace ActionStage

}  // namespace VnV

#endif