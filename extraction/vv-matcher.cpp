
/**
 *  @file vv-matcher.cpp
 *  Clang tool to detect VnV Injection points in the Clang AST. This
 *  Tool searches the AST for calls to the VnV Injection point library and
 *  prints out a json specifying all the injection points in the class.
 *
 *  https://jonasdevlieghere.com/understanding-the-clang-ast/
 */
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <deque>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "json-schema.hpp"

using nlohmann::json;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

#define ANSICOL(code, mod)
#define LAST_RUN_TIME "__LAST_RUN_TIME__"
#define LAST_FILE_NAME "__LAST_FILE_NAME__"

void writeFile(json& cacheInfo, std::string outputFileName,
               std::string cacheFile, std::string packageName,
               std::size_t cacheDataHash, std::string extension, bool writeFiles);

std::pair<std::vector<std::string>, std::size_t> checkCache(
    json& cacheInfo, std::vector<std::string>& file);

json runPreprocessor(CompilationDatabase& comps,
                     std::vector<std::string>& files, std::string packageName_);
json runFinder(CompilationDatabase& db, std::vector<std::string>& files);

/** Apply a custom category to all command-line options so that they are the
 only ones displayed.
*/
static llvm::cl::OptionCategory VnVParserCatagory("VnV Parser Options");

static llvm::cl::opt<std::string> outputFile("output",
                                             llvm::cl::desc("Output file name"),
                                             llvm::cl::value_desc("string"),
                                             llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> extension(
    "extension", llvm::cl::desc("Output file extension"),
    llvm::cl::value_desc("string"), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> cacheFile("cacheFile",
                                            llvm::cl::desc("The cache file"),
                                            llvm::cl::value_desc("file"),
                                            llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> packageName("package",
                                              llvm::cl::desc("PackageName"),
                                              llvm::cl::value_desc("string"),
                                              llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> useCache("useCache", llvm::cl::desc("UseCache"),
                                    llvm::cl::value_desc("bool"),
                                    llvm::cl::init(true),
                                    llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> nowrite ("nowrite", llvm::cl::desc("nowrite"),
                                    llvm::cl::value_desc("bool"),
                                    llvm::cl::init(false),
                                    llvm::cl::cat(VnVParserCatagory));

/**
 * Main Executable for VnV Processor.
 */
int main(int argc, const char** argv) {
  // Parse the options
  CommonOptionsParser OptionsParser(argc, argv, VnVParserCatagory);
  std::string outputFileName = outputFile.getValue();
  if (outputFileName.empty()) {
    throw VnV::VnVExceptionBase("No output file specified");
  }

  std::string packageName_ = packageName.getValue();
  std::hash<std::string> hasher;

  std::string extension_ = extension.getValue();
  if (extension_.empty()) {
    extension_ = "cpp";
  }

  // TODO Add a Cache to this thing. Basically, only want to redo generation
  // if the files have changed. So, put a file json cahce directly in the
  // output file.

  // If the output file exists, load the cache from the comments. The cache
  // can then be used to check if any of the files have changed. We can then
  // update the registriation for only the files that change. That will speed
  // up build times a lot -- especially in cases where the user types make
  // but didnt change eanythin.
  std::string cacheFile_ = cacheFile.getValue();
  if (cacheFile_.empty()) {
    cacheFile_ = outputFileName + ".__cache__";
  }
  std::ifstream cache(cacheFile_.c_str());

  json cacheInfo = json ::object();
  if (cache.good() && useCache.getValue()) {
    cacheInfo = json::parse(cache);
  }
  json& cacheMap = VnV::JsonUtilities::getOrCreate(cacheInfo, "map");
  json& cacheFiles = VnV::JsonUtilities::getOrCreate(cacheInfo, "files");
  json& cacheData = VnV::JsonUtilities::getOrCreate(cacheInfo, "data");

  // Quick loop through the compile commands to remove any command that has
  // the -DVNV_IGNORE=0 command defined. This flag is set by the CMake Build
  // system on files that DEFINETLY don't have any VnV commands or calls. This
  // is used primarily to remove generated files from parsing like the
  // registration files and/or the swig interface files.
  std::vector<std::string> theFiles;
  std::string search = "-DVNV_IGNORE=0";
  for (auto it : OptionsParser.getCompilations().getAllCompileCommands()) {
    auto s = std::find(it.CommandLine.begin(), it.CommandLine.end(), search);
    if (s == it.CommandLine.end()) {
      theFiles.push_back(it.Filename);
    }
  }

  std::pair<std::vector<std::string>, std::size_t> modFiles =
      checkCache(cacheInfo, theFiles);

  if (nowrite.getValue() && modFiles.first.size() == 0 ) {
     std::cout << "No changes detected" << std::endl;
     return 0;
  }

  if (modFiles.first.size() != 0 ) {

    std::cout << "VnV Detected Changes in the following files:\n";
    for (auto it : modFiles.first) {
       std::cout << "\t" << it << std::endl;
    }
    std::cout << "\n\n";
    // Update the Cache
    json vnvDeclares1 = runPreprocessor(OptionsParser.getCompilations(),
                                        modFiles.first, packageName_);

    std::vector<std::string> injectionFiles;
    for (auto it : vnvDeclares1.items()) {
      // update the cache for this file
      json incs = json::array();
      for (auto itt : it.value()["includes"].items()) {
        std::string s = std::to_string(
            hasher(itt.value().get<std::string>()));  // Hash the filename
        if (!cacheMap.contains(s)) {
          cacheMap[s] = itt.value();
        }
        incs.push_back(s);
      }
      cacheFiles[it.key()] = incs;

      json& data = it.value()["data"];
      if (data.contains("InjectionPoints") &&
          data["InjectionPoints"].size() > 0) {
        bool add = false;
        if (packageName_.empty()) {
          add = true;
        } else {
          for (auto itt : data["InjectionPoints"].items()) {
            if (itt.value()["packageName"].get<std::string>() == packageName_) {
              add = true;
              break;
            }
          }
        }
        if (add) {
          injectionFiles.push_back(it.key());
        }
      }
      cacheData[it.key()] = data;

    }
    json found = runFinder(OptionsParser.getCompilations(), injectionFiles);
    std::cout << found.dump(3)<<std::endl;
    // Add the injection point data to the cacheData object.
    for (auto cachedFile : injectionFiles) {
      json& cfileJson = cacheData[cachedFile];
      auto ips = cfileJson.find("InjectionPoints");
      if (ips != cfileJson.end()) {
        for (auto injectionPoint : ips.value().items()) {
          if (packageName_.empty() ||
              packageName_.compare(injectionPoint.value()["packageName"]) ==
                  0) {
            auto info = found.find(injectionPoint.key());
            if (info != found.end()) {
              injectionPoint.value()["parameters"] =
                  (info.value().contains("parameters")
                       ? info.value()["parameters"]
                       : json::array());

              injectionPoint.value()["iterator"] = info.value().value("/iterator"_json_pointer,false);
              json& stages = injectionPoint.value()["stages"];
              for (auto& stage : info.value()["stages"].items()) {
                stages[stage.key()]["info"] = stage.value();
              }
            } else {
              std::cout << "ERROR: Looking for " << injectionPoint.key()
                        << " in " << found.dump(4) << "But could not find it."
                        << std::endl;
              ;
              throw VnV::VnVExceptionBase(
                  "Injection point with no matching parameter info");
            }
          }
        }
      }
    }
  }

  writeFile(cacheInfo, outputFileName, cacheFile_, packageName_,
            modFiles.second, extension_, !nowrite.getValue());
  return 0;
}
