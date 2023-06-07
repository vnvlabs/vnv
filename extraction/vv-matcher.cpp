
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
#include <unistd.h>

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

#include "base/DistUtils.h"
#include "base/Utilities.h"
#include "base/exceptions.h"
#include "json-schema.hpp"

using nlohmann::json;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

#define ANSICOL(code, mod)
#define LAST_RUN_TIME "__LAST_RUN_TIME__"

std::set<std::string>& fexts() {
  static std::set<std::string> fexts_ = {".f90", ".for", ".f", ".fpp", ".i", ".i90", ".ftn", ".F90", ".F"};
  return fexts_;
}

bool isFortranFile(std::string fname) {
  auto s = fname.find_last_of(".");
  if (s != std::string::npos) {
    bool res = (fexts().find(fname.substr(s)) != fexts().end());
    return res;
  }
  return false;
}

void writeFile(json& cacheInfo, std::string outputFileName, std::string targetFileName,
               std::string packageName, std::string strip = "", std::string fortran = "");

void checkCache(json& cacheInfo, std::set<std::string>& cfiles, std::set<std::string>& ffiles);

json runPreprocessor(CompilationDatabase& comps, std::set<std::string>& files);
json runFortranPreprocessor(CompilationDatabase& comps, std::set<std::string>& files);

json runFinder(CompilationDatabase& db, std::vector<std::string>& files, std::string strip);

/** Apply a custom category to all command-line options so that they are the
 only ones displayed.
*/
static llvm::cl::OptionCategory VnVParserCatagory("VnV Parser Options");

static llvm::cl::opt<std::string> outputFile("output", llvm::cl::desc("Output file name"),
                                             llvm::cl::value_desc("string"), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> cacheFile("cache", llvm::cl::desc("The cache file"), llvm::cl::value_desc("file"),
                                            llvm::cl::cat(VnVParserCatagory));


static llvm::cl::opt<std::string> targetFile("targetFile", llvm::cl::desc("The target file"),
                                             llvm::cl::value_desc("file"), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> packageName("package", llvm::cl::desc("PackageName"), llvm::cl::value_desc("string"),
                                              llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> strip("strip", llvm::cl::desc("strip"), llvm::cl::value_desc("string"),
                                        llvm::cl::init(""), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> nostrip("no-strip", llvm::cl::desc("nostrip"), llvm::cl::value_desc("bool"),
                                   llvm::cl::init(false), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> fortranPackage("fortran", llvm::cl::desc("FortranModuleFileName"),
                                                 llvm::cl::value_desc("string"), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> resetCache("reset", llvm::cl::desc("resetCache"), llvm::cl::value_desc("bool"),
                                      llvm::cl::init(false), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> force("force", llvm::cl::desc("force"), llvm::cl::value_desc("bool"), llvm::cl::init(false),
                                 llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> cacheonly("cache-only", llvm::cl::desc("cache-only"), llvm::cl::value_desc("bool"),
                                     llvm::cl::init(false), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> fixomp("fix-omp", llvm::cl::desc("fix-omp"), llvm::cl::value_desc("bool"),
                                  llvm::cl::init(false), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> inject("inject", llvm::cl::desc("inject"), llvm::cl::value_desc("bool"),
                                  llvm::cl::init(false), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> outputonly("output-only", llvm::cl::desc("output-only"), llvm::cl::value_desc("bool"),
                                      llvm::cl::init(false), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::list<std::string> ignoreDir("ignore-dir", llvm::cl::desc("ignore-dir"), llvm::cl::value_desc("string"),
                                             llvm::cl::cat(VnVParserCatagory));

static llvm::cl::list<std::string> ignoreFiles("ignore-file", llvm::cl::desc("ignore-files"),
                                               llvm::cl::value_desc("string"), llvm::cl::cat(VnVParserCatagory));

static llvm::cl::list<std::string> extraIncludes("extra-inc", llvm::cl::desc("extra-inc"),
                                                 llvm::cl::value_desc("string"), llvm::cl::cat(VnVParserCatagory));

bool acceptFile(std::string filename) {
  for (auto& it : ignoreDir) {
    if (VnV::DistUtils::fileInDirectory(filename, it)) {
      return false;
    }
  }
  for (auto& it : ignoreFiles) {
    if (VnV::DistUtils::fileEquals(filename, it)) {
      return false;
    }
  }

  return true;
}

void fix_omp(std::string compil_dbase) {
  const char* nn = getenv("OPENMP_INCLUDE_DIR");

  if (nn != nullptr) {
    // Last argument in the command line is the compile_database file
    // This is a bug fix that adds the openmp include directory to any
    // file that lists -fopenmp

    // Scope is used here to close the ifstream and get rid of it
    // from the namespace of the function so we cant accidently use
    // it again.
    nlohmann::json j;
    {
      std::ifstream ifs(compil_dbase);
      j = json::parse(ifs);
    }

    // Dont remove the space in front of the "I" please.
    std::string ss(nn);
    ss = " -I" + ss;

    for (auto& command : j.items()) {
      auto& comp = command.value();  // Object

      if (comp.contains("command")) {
        std::string comm = comp["command"].get<std::string>();
        size_t pos;  // position of string b
        pos = comm.find("-fopenmp");
        if (pos != std::string::npos && comm.find(ss) == std::string::npos) {
          comm.insert(pos + 8, ss);
          comp["command"] = comm;
        }
      } else if (comp.contains("arguments")) {
        auto& comm = comp["arguments"];  // Array
        auto it = std::find(comm.begin(), comm.end(), "-fopenmp");
        if (it != comm.end() && std::find(it, comm.end(), ss.substr(1)) == comm.end()) {
          comm.insert(it + 1, ss.substr(1));
        }
      } else {
        // std::abort();  // Invalid
      }
    }

    std::ofstream ofs(compil_dbase);
    ofs << j.dump(4);
  } else {
    std::cout << "export OPENMP_INCLUDE_DIR=<path> prior to calling with --fix-omp option" << std::endl;
  }
}

/**
 * Main Executable for VnV Processor.
 */
int main(int argc, const char** argv) {
  try {
    for (int i = 0; i < argc; i++) {
      if (strcmp(argv[i], "--fix-omp") == 0) {
        fix_omp(argv[argc - 1]);
      }
    }

    // Parse the options
    CommonOptionsParser OptionsParser(argc, argv, VnVParserCatagory);

    // Strip val is "" by default -- If no-strip is passed, we set it empty, else, we set
    // it to the current working dir.
    std::string stripVal = strip.getValue();
    if (stripVal.size() == 0 && !nostrip.getValue()) {
      char* a = get_current_dir_name();
      stripVal = a;
      free(a);
    }

    std::string outputFileName = outputFile.getValue();
    if (!cacheonly.getValue() && outputFileName.empty()) {
      HTHROW INJECTION_EXCEPTION_("No output file specified");
    }

    if (cacheonly.getValue() && outputonly.getValue()) {
      HTHROW INJECTION_EXCEPTION_("Cant have cache only and output only at the same time. ");
    }

    std::string targetFileName = targetFile.getValue();

    std::hash<std::string> hasher;

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
        } catch (std::exception& e) {
          std::cerr << "Invalid Cache file: " << e.what() << std::endl;
          cacheInfo = json::object();
        }
      }
    }

    json& cacheMap = VnV::JsonUtilities::getOrCreate(cacheInfo, "map");
    json& cacheFiles = VnV::JsonUtilities::getOrCreate(cacheInfo, "files");
    json& cacheData = VnV::JsonUtilities::getOrCreate(cacheInfo, "data");

    if (!outputonly.getValue()) {
      // Quick loop through the compile commands to remove any command that has
      // the -DVNV_IGNORE=0 command defined. This flag is set by the CMake Build
      // system on files that DEFINETLY don't have any VnV commands or calls. This
      // is used primarily to remove generated files from parsing like the
      // registration files and/or the swig interface files.
      std::set<std::string> theFiles, fortranFiles, allfiles;
      std::string search = "-DVNV_IGNORE=0";

      for (auto it : OptionsParser.getCompilations().getAllCompileCommands()) {
        auto s = std::find(it.CommandLine.begin(), it.CommandLine.end(), search);
        if (s == it.CommandLine.end()) {
          std::string fname;
          if (it.Filename[0] != '/') {
            fname = it.Directory + "/" + it.Filename;
          } else {
            fname = it.Filename;
          }

          if (isFortranFile(fname)) {
            fortranFiles.insert(fname);
          } else {
            theFiles.insert(fname);
          }
          allfiles.insert(fname);
        }
      }

      // This checks if any of the cpp files have changed and if any of the
      // headers included in that cpp file ON THE LAST RUN have changed.
      checkCache(cacheInfo, theFiles, fortranFiles);

      if (theFiles.size() == 0) {
        std::cout << "---->No C/C++ file changes detected" << std::endl;
      } else {
        // Update the Cache
        json vnvDeclares1 = runPreprocessor(OptionsParser.getCompilations(), theFiles);

        std::vector<std::string> injectionFiles;
        for (auto it : vnvDeclares1.items()) {
          // update the cache for this file
          json incs = json::array();
          for (auto itt : it.value()["includes"].items()) {
            std::string s = std::to_string(hasher(itt.value().get<std::string>()));  // Hash the filename
            if (!cacheMap.contains(s)) {
              cacheMap[s] = itt.value();
            }
            incs.push_back(s);
          }
          cacheFiles[it.key()] = incs;

          json& data = it.value()["data"];
          if (data.contains("InjectionPoints") && data["InjectionPoints"].size() > 0) {
            injectionFiles.push_back(it.key());
          }
          cacheData[it.key()] = data;
        }

        json found = runFinder(OptionsParser.getCompilations(), injectionFiles, stripVal);

        // Add the injection point data to the cacheData object.
        for (auto cachedFile : injectionFiles) {
          // This is the data from the cache extract during preprocessing for this file.
          json& cfileJson = cacheData[cachedFile];

          // This is the preprocessor injection points
          auto ips = cfileJson.find("InjectionPoints");

          if (ips != cfileJson.end()) {
            // Iterate over all the injection points found in this file by the preprocessor.
            for (auto injectionPoint : ips.value().items()) {
              // Try and find the parameters as extracted by the clang tool.
              auto info = found.find(injectionPoint.key());

              if (info != found.end()) {
                injectionPoint.value()["parameters"] =
                    (info.value().contains("parameters") ? info.value()["parameters"] : json::array());

                json& stages = injectionPoint.value()["stages"];
                for (auto& stage : info.value()["stages"].items()) {
                  stages[stage.key()]["info"] = stage.value();
                }
              } else {
                std::cout << "Could Not Find Matching Parameters " << injectionPoint.key() << std::endl;
              }
            }
          }
        }
      }

      if (fortranFiles.size() == 0) {
        std::cout << "---->No FORTRAN file changes detected" << std::endl;
      } else {
        json fortranInfo = runFortranPreprocessor(OptionsParser.getCompilations(), fortranFiles);
        // Update the fortran cache information.
        for (auto itt : fortranFiles) {
          std::string s = std::to_string(hasher(itt));  // Hash the filename
          if (!cacheMap.contains(s)) {
            cacheMap[s] = itt;
          }
          cacheFiles[itt] = {s};
          cacheData[itt] = fortranInfo[itt];
        }
      }

      // Update the Cache file
      if (cacheFile.empty()) {
        std::cout << "---->No Cache File Name provided..... Skipping writing cache " << cacheFile << std::endl;
      } else {
        std::ofstream cacheStream(cacheFile);

        if (!cacheStream.good()) {
          std::cout << "---->Cache File name invalid..... Skipping writing of cache." << cacheFile << std::endl;
        } else {
          // Write the cache.
          std::cout << "---->Updating the cache file" << std::endl;
          // Do some cleaning of the data for size.
          json& d = cacheInfo["data"];
          for (auto it = d.begin(); it != d.end();) {
            if (it.value().is_object() && it.value().size() == 0) {
              it = d.erase(it);
            } else {
              ++it;
            }
          }

          // get a list of all includes used in the different files.
          std::hash<std::string> hasher;
          std::set<std::string> used;
          for (auto it : cacheInfo["files"].items()) {
            for (auto itt : it.value().items()) {
              used.insert(itt.value().get<std::string>());
            }
            used.insert(std::to_string(hasher(it.key())));
          }

          // remove any files from the map that are not used.
          json& m = cacheInfo["map"];
          for (auto it = m.begin(); it != m.end();) {
            if ((used.end() == used.find(it.key()))) {
              it = m.erase(it);
            } else {
              ++it;
            }
          }

          time_t now;
          time(&now);
          cacheInfo[LAST_RUN_TIME] = VnV::TimeUtils::timeToISOString(&now);
          cacheStream << cacheInfo.dump();
          cacheStream.close();
        }
      }
    }

    // If we did not set cacheonly , write the file.
    if (!cacheonly.getValue()) {
      // Write the file.

      writeFile(cacheInfo, outputFileName, targetFileName, packageName.getValue(), strip.getValue(),
                fortranPackage.getValue());
    }

    return 0;

  } catch (std::exception& e) {
    std::cout << "An Error Occurred: " << e.what() << std::endl;
    std::abort();
  }
}
