
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

void writeFile(json& cacheInfo, std::string outputFileName,
               std::string cacheFile, std::string packageName, bool force);

std::set<std::string> checkCache(json& cacheInfo, std::set<std::string>& file);

json runPreprocessor(CompilationDatabase& comps, std::set<std::string>& files);

json runFinder(CompilationDatabase& db, std::vector<std::string>& files);

/** Apply a custom category to all command-line options so that they are the
 only ones displayed.
*/
static llvm::cl::OptionCategory VnVParserCatagory("VnV Parser Options");

static llvm::cl::opt<std::string> outputFile("output",
                                             llvm::cl::desc("Output file name"),
                                             llvm::cl::value_desc("string"),
                                             llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> cacheFile("cache",
                                            llvm::cl::desc("The cache file"),
                                            llvm::cl::value_desc("file"),
                                            llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> packageName("package",
                                              llvm::cl::desc("PackageName"),
                                              llvm::cl::value_desc("string"),
                                              llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> resetCache("reset", llvm::cl::desc("resetCache"),
                                      llvm::cl::value_desc("bool"),
                                      llvm::cl::init(false),
                                      llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> force("force", llvm::cl::desc("force"),
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

  std::cout << "\n"
            << "********************************************************\n"
            << "-->Starting VnV Extraction for " << packageName << "\n"; 

  // If the output file exists, load the cache from the comments. The cache
  // can then be used to check if any of the files have changed. We can then
  // update the registriation for only the files that change. That will speed
  // up build times a lot -- especially in cases where the user types make
  // but didnt change eanythin.
  std::string cacheFile_ = cacheFile.getValue();
  json cacheInfo = json ::object();

  if (!resetCache.getValue() && !cacheFile_.empty()) {
    std::ifstream cache(cacheFile_.c_str());
    if (cache.good()) {
      try {
        cacheInfo = json::parse(cache);
      } catch (...) {
        std::cerr << "Invalid Cache file" << std::endl;
        cacheInfo = json::object();
      }
    }
  }

  json& cacheMap = VnV::JsonUtilities::getOrCreate(cacheInfo, "map");
  json& cacheFiles = VnV::JsonUtilities::getOrCreate(cacheInfo, "files");
  json& cacheData = VnV::JsonUtilities::getOrCreate(cacheInfo, "data");

  // Quick loop through the compile commands to remove any command that has
  // the -DVNV_IGNORE=0 command defined. This flag is set by the CMake Build
  // system on files that DEFINETLY don't have any VnV commands or calls. This
  // is used primarily to remove generated files from parsing like the
  // registration files and/or the swig interface files.
  std::set<std::string> theFiles;
  std::string search = "-DVNV_IGNORE=0";
  for (auto it : OptionsParser.getCompilations().getAllCompileCommands()) {
    auto s = std::find(it.CommandLine.begin(), it.CommandLine.end(), search);
    if (s == it.CommandLine.end()) {
      if (it.Filename[0] != '/') {
        theFiles.insert(it.Directory + "/" + it.Filename);
      } else {
        theFiles.insert(it.Filename);
      }
    }
  }

  // TODO This checks if any of the cpp files have changed and if any of the
  // headers included in that cpp file ON THE LAST RUN have changed. To get
  // everything, we need to go ahead and generate a full list of all headers.
  // Otherwise we wont catch changes in new header files.
  std::set<std::string> modFiles = checkCache(cacheInfo, theFiles);

  if (modFiles.size() == 0) {
    std::cout << "---->No file changes detected" << std::endl;
  } else {
    
    // Update the Cache
    json vnvDeclares1 =
        runPreprocessor(OptionsParser.getCompilations(), modFiles);

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
        injectionFiles.push_back(it.key());
      }
      cacheData[it.key()] = data;
    }

    json found = runFinder(OptionsParser.getCompilations(), injectionFiles);

    // Add the injection point data to the cacheData object.
    for (auto cachedFile : injectionFiles) {
      json& cfileJson = cacheData[cachedFile];

      auto ips = cfileJson.find("InjectionPoints");

      if (ips != cfileJson.end()) {
        for (auto injectionPoint : ips.value().items()) {
          auto info = found.find(injectionPoint.key());

          if (info != found.end()) {
            injectionPoint.value()["parameters"] =
                (info.value().contains("parameters")
                     ? info.value()["parameters"]
                     : json::array());

            // injectionPoint.value()["iterator"] =
            // info.value().value("/iterator"_json_pointer,false);
            json& stages = injectionPoint.value()["stages"];
            for (auto& stage : info.value()["stages"].items()) {
              stages[stage.key()]["info"] = stage.value();
            }
          }
        }
      }
    }
  }

  writeFile(cacheInfo, outputFileName, cacheFile_, packageName_,
            force.getValue());

 std::cout  << "-->Finished VnV Extraction for " << packageName << "\n"
            << "********************************************************\n\n";

  return 0;
}
