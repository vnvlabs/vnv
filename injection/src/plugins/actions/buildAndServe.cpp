#include <Python.h>

#include <iostream>
#include <string>

#include "base/DistUtils.h"
#include "base/stores/OutputEngineStore.h"
#include "interfaces/IAction.h"

namespace {

struct ReportingInfo {
  bool run;
  std::string author;
  std::string project;
  std::string version;
  std::string release;
  std::string directory;
};

const char* getSchema() {
  static std::string s = R"(
    {
       "type" : "object",
       "properties" : {
         "on" : {"type" : "boolean", "default" : true },
         "author" : {"type" : "string" , "default" : "$USER" },
         "directory" : { "type" : "string" , "default" : "./docs"},
         "title" : {"type" : "string" , "default" : "VnV Simulation" },
         "version" : {"type" : "string" , "default" : "0" },
         "release" : {"type" : "string" , "default" : "0" }
       }
    }
    )";
  return s.c_str();
}

ReportingInfo parse(json& j) {
  ReportingInfo reportingInfo;
  reportingInfo.run = false;

#define InfMap(prop, key, type, def) \
  reportingInfo.prop = j.contains(key) ? j[key].get<type>() : def
  InfMap(run, "on", bool, true);
  InfMap(author, "author", std::string, "anon");
  InfMap(directory, "directory", std::string, "docs");
  InfMap(project, "title", std::string, "Development");
  InfMap(version, "version", std::string, "0");
  InfMap(release, "release", std::string, "0");
#undef InfMap
  return reportingInfo;
}

void generateReport(const ReportingInfo& reportConfig) {
  if (reportConfig.run) {
    bool weInitializedPython = !Py_IsInitialized();
    if (weInitializedPython) {
      Py_Initialize();
    }

    std::string reader =
        VnV::OutputEngineStore::instance().getEngineManager()->getKey();
    std::string file = VnV::OutputEngineStore::instance()
                           .getEngineManager()
                           ->getMainFilePath();

    std::ostringstream oss;
    oss << "import sys\ntry:\n  from vnv import quickstart\nexcept "
           "ModuleNotFoundError as err:\n  print(err)\n  sys.exit(1)\n";
    oss << "quickstart(";
    oss << "\"" << reader << "\"";
    oss << ",\"" << file << "\"";
    oss << ",version=\"" << reportConfig.version << "\"";
    oss << ",release=\"" << reportConfig.release << "\"";
    oss << ",author=\"" << reportConfig.author << "\"";
    oss << ",title=\"" << reportConfig.project << "\"";
    oss << ",path=\"" << reportConfig.directory << "\"";
    oss << ",builder=\"html\"";
    oss << ",builder_dir=\"_build\"";
    oss << ")";
    PyRun_SimpleString(oss.str().c_str());

    bool serve = true;
    if (serve) {
      std::string builddir = VnV::DistUtils::join(
          {reportConfig.directory, "_build", "html"}, 0777, false);

      std::ostringstream oss1;
      oss1 << "import sys\ntry:\n  from vnv import serve\nexcept "
              "ModuleNotFoundError as err:\n  print(err)\n  sys.exit(1)\n";
      oss1 << "serve(";
      oss1 << "directory=\"" << builddir << "\"";
      oss1 << ")";
      PyRun_SimpleString(oss1.str().c_str());
    }
    if (weInitializedPython) {
      Py_FinalizeEx();
    }
  }
}

}  // namespace

INJECTION_ACTION_S(VNVPACKAGENAME, BuildAndServe, getSchema()) {
  generateReport(parse(config));
}