
/**
 *  @file vv-matcher.cpp
 *  Clang tool to detect VnV Injection points in the Clang AST. This
 *  Tool searches the AST for calls to the VnV Injection point library and
 *  prints out a json specifying all the injection points in the class.
 *
 *  https://jonasdevlieghere.com/understanding-the-clang-ast/
 */
#include <sys/stat.h>

#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <string>
#include <vector>

#include "base/Utilities.h"
#include "base/exceptions.h"
#include "clang/AST/RawCommentList.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/CommentOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "json-schema.hpp"
#include "llvm/Support/CommandLine.h"

using nlohmann::json;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

#define ANSICOL(code, mod)

#define LAST_RUN_TIME "__LAST_RUN_TIME__"
#define LAST_FILE_NAME "__LAST_FILE_NAME__"

namespace {

void jsonAdder(std::string packageName, json& input, json& output,
               std::string key) {
  for (auto test : input[key].items()) {
    test.value()["packageName"] = packageName;
    output[key][packageName + ":" + test.key()] = test.value();
  }
}
std::string timeToISOString(time_t* t) {
  char buf[80];
  std::strftime(buf, 80, "%Y-%m-%d-%H-%M-%S", std::gmtime(t));
  return buf;
}

std::string timeForFile(std::string filename) {
  struct stat result;
  stat(filename.c_str(), &result);
  return timeToISOString(&result.st_mtime);
}

// Copied shamelessly from an anonomous namespace in the Clang source code in
// RawCommentList.cpp
static bool onlyWhitespaceBetween(SourceManager& SM, SourceLocation Loc1,
                                  SourceLocation Loc2,
                                  unsigned MaxNewlinesAllowed) {
  std::pair<FileID, unsigned> Loc1Info = SM.getDecomposedLoc(Loc1);
  std::pair<FileID, unsigned> Loc2Info = SM.getDecomposedLoc(Loc2);
  // Question does not make sense if locations are in different files.
  if (Loc1Info.first != Loc2Info.first) return false;

  bool Invalid = false;
  const char* Buffer = SM.getBufferData(Loc1Info.first, &Invalid).data();
  if (Invalid) return false;

  unsigned NumNewlines = 0;
  assert(Loc1Info.second <= Loc2Info.second && "Loc1 after Loc2!");
  // Look for non-whitespace characters and remember any newlines seen.
  for (unsigned I = Loc1Info.second; I != Loc2Info.second; ++I) {
    switch (Buffer[I]) {
    default:
      return false;
    case ' ':
    case '\t':
    case '\f':
    case '\v':
      break;
    case '\r':
    case '\n':
      ++NumNewlines;

      // Check if we have found more than the maximum allowed number of
      // newlines.
      if (NumNewlines > MaxNewlinesAllowed) return false;

      // Collapse \r\n and \n\r into a single newline.
      if (I + 1 != Loc2Info.second &&
          (Buffer[I + 1] == '\n' || Buffer[I + 1] == '\r') &&
          Buffer[I] != Buffer[I + 1])
        ++I;
      break;
    }
  }

  return true;
}

std::string stringArgs(Preprocessor& pp, const Token* token,
                       bool spaceBeforeLast) {
  std::string s = "";
  while (token->isNot(tok::eof)) {
    if (spaceBeforeLast &&
        (pp.getSpelling(*(token + 1)) == "," || (token + 1)->is(tok::eof)))
      s += " ";
    s += pp.getSpelling(*token);
    token++;
  }
  return s;
}

std::map<std::string, std::string> parseArgs(Preprocessor& pp,
                                             const Token* token) {
  return VnV::StringUtils::variadicProcess(stringArgs(pp, token, true).c_str());
}

json& getOrCreate(json& j, std::string name) {
  if (!j.contains(name)) {
    json jj;
    j[name] = json::object();
  }
  return j[name];
}

std::string getValueFromStringLiteral(const Expr* a) {
  std::string xx = a->getStmtClassName();
  if (xx == "StringLiteral") {
    return ((const clang::StringLiteral*)a)->getString().str();
  } else {
    llvm::errs() << "Error Not a String Literal ";
    a->dump();
    return "";
  }
}

unsigned int getInfo(const CallExpr* call, const FunctionDecl* func,
                     const MatchFinder::MatchResult& Result, json& info,
                     std::string& id, std::string& filename, int begin) {
  FullSourceLoc callLocation = Result.Context->getFullLoc(call->getBeginLoc());
  FullSourceLoc funclocation = Result.Context->getFullLoc(func->getBeginLoc());

  info["lineNumber"] = callLocation.getSpellingLineNumber();
  info["lineColumn"] = callLocation.getSpellingColumnNumber();

  SourceManager& srcMgr = Result.Context->getSourceManager();
  filename = srcMgr.getFilename(func->getLocation()).str();
  info["filename"] =
      filename;  // Result.SourceManager->getFilename(callLocation);

  SourceManager& manager(Result.Context->getSourceManager());
  info["Calling Function"] = func->getNameInfo().getAsString();
  info["Calling Function Line"] = funclocation.getSpellingLineNumber();
  info["Calling Function Column"] = funclocation.getSpellingColumnNumber();

  json parameters;
  unsigned int count = (begin);
  std::string package = VnV::StringUtils::trim_copy(
      getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts()));
  id = VnV::StringUtils::trim_copy(
      getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts()));
  std::string key = package + ":" + id;
  id = key;
  if (begin > 0) count++;
  return count;
}

bool extractParameters(const CallExpr* E, unsigned int count,
                       json& parameters) {
  bool ret = false;
  while (count < E->getNumArgs() - 1) {
    const clang::Expr* a = E->getArg(count++)->IgnoreParenCasts();
    std::string xs = getValueFromStringLiteral(a);
    const clang::Expr* aa = E->getArg(count++)->IgnoreParenCasts();

    if (aa->getType()->isDependentType()) ret = true;

    std::string ax = VnV::StringUtils::squash_copy(aa->getType().getAsString());
    parameters[xs] = ax;
  }
  return ret;
}

void addParameters(const CallExpr* E, json& idJson, unsigned int count) {
  json params;
  if (extractParameters(E, count, params)) {
    idJson["templateParameters"] = params;
  } else {
    json& possibleParams = VnV::JsonUtilities::getOrCreate(
        idJson, "parameters", VnV::JsonUtilities::CreateType::Array);
    possibleParams.push_back(params);
  }
}

}  // namespace

class VnVPrinter : public MatchFinder::MatchCallback {
 private:
  json main_json;

 public:
  VnVPrinter() { main_json = json::object(); }

  json& get() { return main_json; }

  virtual void run(const MatchFinder::MatchResult& Result) {
    std::string filename;
    std::string id;
    json info;

    const FunctionDecl* FF =
        Result.Nodes.getNodeAs<clang::FunctionDecl>("function");
    if (const CallExpr* E =
            Result.Nodes.getNodeAs<clang::CallExpr>("callsite")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 2);
      json& ip_json = getOrCreate(main_json, filename);
      json& idJson = getOrCreate(ip_json, id);
      json& singleJson = getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);
    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("callsite_begin")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 2);
      json& ip_json = getOrCreate(main_json, filename);
      json& idJson = getOrCreate(ip_json, id);
      json& singleJson = getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);
    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("cpp_callsite")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      json& ip_json = getOrCreate(main_json, filename);
      json& idJson = getOrCreate(ip_json, id);
      json& singleJson = getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);
    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>(
                   "cpp_callsite_begin")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      json& ip_json = getOrCreate(main_json, filename);
      json& idJson = getOrCreate(ip_json, id);
      json& singleJson = getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);
    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("callsite_iter")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 0);
      json& ip_json = getOrCreate(main_json, filename);
      std::string iterid = VnV::StringUtils::trim_copy(
          getValueFromStringLiteral(E->getArg(count++)->IgnoreParenCasts()));
      json& idJson = getOrCreate(ip_json, id);
      json& stagesJson = getOrCreate(idJson, "stages");
      stagesJson[iterid] = info;
    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("callsite_end")) {
      getInfo(E, FF, Result, info, id, filename, 0);
      json& ip_json = getOrCreate(main_json, filename);
      json& idJson = getOrCreate(ip_json, id);
      json& singleJson = getOrCreate(idJson, "stages");
      singleJson["End"] = info;
    }
  }
};

class RegistrationWriter {
 public:
  std::map<std::string, std::ostringstream> oss_register;
  std::map<std::string, std::ostringstream> oss_declare;
  std::map<std::string, json> pjson;
  bool finalized = false;

  void createPackageOss(std::string packageName) {
    if (oss_register.find(packageName) == oss_register.end()) {
      std::ostringstream os_declare;
      os_declare << "#define PACKAGENAME " << packageName << "\n";
      os_declare << "#include \"VnV.h\" \n";
      oss_declare[packageName] = std::move(os_declare);

      std::ostringstream os_register;
      os_register << "INJECTION_REGISTRATION(){\n";
      oss_register[packageName] = std::move(os_register);

      pjson[packageName] = json::object();
    }
  }

  void finalizePackage(std::string packageName) {
    auto it = oss_register.find(packageName);
    if (it != oss_register.end()) {
      it->second << "\tREGISTER_FULL_JSON(getFullRegistrationJson_" << it->first
                 << ");\n";
      it->second << "};\n\n";
      oss_declare[packageName]
          << "const char* getFullRegistrationJson_" << it->first << "(){"
          << "\n"
          << "\t return "
          << VnV::StringUtils::escapeQuotes(pjson[it->first].dump(), true)
          << ";}\n\n"
          << it->second.str() << "\n\n";
    }
  }

  void finalizeAllPackages() {
    if (!finalized) {
      for (auto& it : oss_register) {
        finalizePackage(it.first);
      }
    }
    finalized = true;
  }

  std::string printPackage(std::string packageName) {
    if (!packageName.empty()) {
      auto it = oss_declare.find(packageName);
      if (it != oss_declare.end()) {
        return it->second.str();
      } else {
        // This means the package includes VnV.h, but does not have any VnV
        // INJECTION CALLS. So, we should return a empty configuration.
        createPackageOss(packageName);
        finalizePackage(packageName);
        return oss_declare[packageName].str();
      }
    }
    // Otherwise, create a json object containing all of them.
    json j = json::object();
    for (auto& it : oss_declare) {
      j[it.first] = it.second.str();
    }
    return j.dump();
  }

  void registerHelper(json& j, std::string key, std::string reg,
                      std::string pname) {
    if (j.contains(key)) {
      for (auto it : j[key].items()) {
        std::string packageName = it.value()["packageName"];
        if (pname.empty() || packageName == pname) {
          std::string name = it.value()["name"];
          createPackageOss(packageName);

          // Save the json, declare the engine, register the engine.
          it.value().erase("name");
          it.value().erase("packageName");
          getOrCreate(pjson[packageName], key)[name] = it.value();

          oss_declare[packageName] << "DECLARE" << reg << "(" << name << ")\n";
          oss_register[packageName] << "\tREGISTER" << reg << "(" << name
                                    << ");\n";
        }
      }
    }
  }

  RegistrationWriter(json& j, std::string packageName = "") {
    registerHelper(j, "Tests", "TEST", packageName);
    registerHelper(j, "Engines", "ENGINE", packageName);
    registerHelper(j, "UnitTests", "UNITTEST", packageName);
    registerHelper(j, "Serializers", "SERIALIZER", packageName);
    registerHelper(j, "Transforms", "TRANSFORM", packageName);
    registerHelper(j, "Comms", "COMM", packageName);
    registerHelper(j, "Reducers", "REDUCER", packageName);
    registerHelper(j, "DataTypes", "DATATYPE", packageName);

    if (j.contains("LogLevels")) {
      for (auto it : j["LogLevels"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();

        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();
          std::string c = it.value()["color"].get<std::string>();
          createPackageOss(pname);
          oss_register[pname] << "\tREGISTERLOGLEVEL(" << n << "," << c
                              << ");\n";
          getOrCreate(pjson[packageName], "LogLevels")[n] = it.value();
        }
      }
    }

    if (j.contains("SubPackages")) {
      for (auto it : j["SubPackages"].items()) {
        std::string pname = it.value()["packageName"].get<std::string>();
        if (packageName.empty() || pname == packageName) {
          std::string n = it.value()["name"].get<std::string>();
          createPackageOss(pname);
          oss_declare[pname] << "DECLARESUBPACKAGE(" << n << ")\n";
          oss_register[pname] << "\tREGISTERSUBPACKAGE(" << n << ");\n";
          getOrCreate(pjson[packageName], "SubPackages")[n] = it.value();
        }
      }
    }

    // Catch the options. They are a little different from above (no name
    // parameter).
    if (j.contains("Options")) {
      for (auto it : j["Options"].items()) {
        std::string pname = it.key();
        if (packageName.empty() || pname == packageName) {
          createPackageOss(pname);
          oss_declare[pname] << "DECLAREOPTIONS\n";
          oss_register[pname] << "\tREGISTEROPTIONS\n";
          pjson[pname]["Options"] = it.value();
        }
        if (packageName == pname) break;
      }
    }
    if (j.contains("Communicator")) {
      for (auto it : j["Communicator"].items()) {
        std::string pname = it.key();
        if (packageName.empty() || pname == packageName) {
          createPackageOss(pname);
          oss_register[pname]
              << "\tVnV_Declare_Communicator(\"" << pname << "\",\""
              << it.value()["package"].get<std::string>() << "\",\""
              << it.value()["name"].get<std::string>() << "\");\n";
          pjson[pname]["Communicator"] = it.value();
        }
        if (packageName == pname) break;
      }
    }

    // Catch the injection points
    if (j.contains("InjectionPoints")) {
      for (auto it : j["InjectionPoints"].items()) {
        std::string pname = it.value()["packageName"];
        if (packageName.empty() || pname == packageName) {
          std::string name = it.value()["name"];
          json& params =
              it.value()["parameters"]
                        [0];  // TODO TEMPLATE ARGUEMENTS NOT SUPPORTED ?
          pjson[pname]["InjectionPoints"][name] = it.value();

          createPackageOss(pname);
          std::string escaped =
              VnV::StringUtils::escapeQuotes(params.dump(), true);
          oss_register[pname] << "\tRegister_Injection_Point(" << name << ","
                              << escaped << ");\n";
        }
      }
    }
    finalizeAllPackages();
  }
};

/** Apply a custom category to all command-line options so that they are the
 only ones displayed.
*/
static llvm::cl::OptionCategory VnVParserCatagory("VnV Parser Options");

static llvm::cl::opt<std::string> outputFile("output",
                                             llvm::cl::desc("Output file name"),
                                             llvm::cl::value_desc("file"),
                                             llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> cacheFile("cacheFile",
                                            llvm::cl::desc("The cache file"),
                                            llvm::cl::value_desc("file"),
                                            llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<std::string> packageName("package",
                                              llvm::cl::desc("PackageName"),
                                              llvm::cl::value_desc("string"),
                                              llvm::cl::cat(VnVParserCatagory));

static llvm::cl::opt<bool> useCache("useCache",
                                   llvm::cl::desc("UseCache"),
                                   llvm::cl::value_desc("bool"),
                                   llvm::cl::init(true),
                                   llvm::cl::cat(VnVParserCatagory));

namespace {

class PreprocessCallback : public PPCallbacks, CommentHandler {
 public:
  json& thisJson;
  CommentOptions commentOptions;
  Preprocessor& pp;
  std::set<std::string>& modTime;
  std::unique_ptr<RawComment> currComment;
  SourceLocation currentLoc;
  std::string packName;
  bool active = false;

  PreprocessCallback(std::string packageName, json& j,
                     std::set<std::string>& includes, Preprocessor& PP)
      : PPCallbacks(), pp(PP), thisJson(j), modTime(includes) {
    commentOptions.ParseAllComments = false;
    packName = packageName;
    // PP.addCommentHandler(this);
  }

  std::string getCommentFor(DiagnosticsEngine& eng, SourceManager& SM,
                            SourceLocation loc) {
    if (currComment == nullptr) return "";

    if (onlyWhitespaceBetween(SM, currentLoc, SM.getExpansionLoc(loc), 1000)) {
      std::string s = currComment->getFormattedText(SM, eng);
      currComment.reset(nullptr);
      return s;
    }
    return "";
  }

  bool HandleComment(Preprocessor& PP, SourceRange Comment) override {
    SourceManager& SM = PP.getSourceManager();
    currComment.reset(new RawComment(SM, Comment, commentOptions, false));
    currentLoc = Comment.getEnd();
    if (!currComment->isDocumentation()) {
      currComment.reset(nullptr);
    }
    return false;
  }

  json& getDef(std::string type, const Token* name) {
    std::string pname = thisJson["packageName"];
    std::string nstr = pp.getSpelling(*name);
    json& j = getOrCreate(thisJson, type);
    json& jj = getOrCreate(j, pname + ":" + nstr);
    jj["name"] = nstr;
    jj["packageName"] = pname;
    return jj;
  }

  std::string getDocs(SourceRange Range) {
    return getCommentFor(pp.getDiagnostics(), pp.getSourceManager(),
                         Range.getBegin());
  }

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType, FileID PrevFID) override {
    std::string fname = pp.getSourceManager().getFilename(Loc).str();
    if (!fname.empty()) {
      auto f = pp.getSourceManager().getFileManager().getFileRef(fname);
      if (!f) {
        modTime.insert(fname);  // = f->getModificationTime();
      }
    }
  }

  void MacroExpands(const Token& MacroNameTok, const MacroDefinition& MD,
                    SourceRange Range, const MacroArgs* Args) override {
    if (!active) return;

    std::string nae = pp.getSpelling(MacroNameTok);

    if (nae.substr(0, 10).compare("INJECTION_") != 0) return;

    if (nae == "INJECTION_TEST_RS") {
      json& jj = getDef("Tests", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
      jj["parameters"] = parseArgs(pp, Args->getUnexpArgument(3));
    } else if (nae == "INJECTION_SERIALIZER_R") {
      json& jj = getDef("Serializers", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
      jj["type"] = stringArgs(pp, Args->getUnexpArgument(2), false);
    } else if (nae == "INJECTION_TRANSFORM_R") {
      json& jj = getDef("Transforms", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
      jj["to"] = stringArgs(pp, Args->getUnexpArgument(2), false);
      jj["from"] = stringArgs(pp, Args->getUnexpArgument(3), false);
    } else if (nae == "INJECTION_OPTIONS") {
      json& jj = getOrCreate(thisJson, "Options");
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_LOGLEVEL") {
      json& jj = getDef("LogLevels", Args->getUnexpArgument(0));
      jj["color"] = pp.getSpelling(*Args->getUnexpArgument(1));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_SUBPACKAGE") {
      json& jj = getDef("SubPackages", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_UNITTEST_R" ||
               nae == "INJECTION_UNITTEST_RAW") {
      json& jj = getDef("UnitTests", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_ENGINE") {
      json& jj = getDef("Engines", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_DATATYPE") {
      json& jj = getDef("DataTypes", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_REDUCER") {
      json& jj = getDef("Reducers", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_COMM") {
      json& jj = getDef("Comms", Args->getUnexpArgument(0));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_COMMUNICATOR") {
      json& jj = getOrCreate(thisJson, "Communicator");
      jj["docs"] = getDocs(Range);
      jj["package"] = pp.getSpelling(*Args->getUnexpArgument(0));
      jj["name"] = pp.getSpelling(*Args->getUnexpArgument(1));
    } else if (nae == "INJECTION_POINT_C" || nae == "INJECTION_LOOP_BEGIN_C") {
      json& jj = getDef("InjectionPoints", Args->getUnexpArgument(1));
      json& stages = getOrCreate(jj, "stages");
      json& thisStage = getOrCreate(stages, "Begin");
      jj["docs"] = getDocs(Range);
      thisStage["docs"] = "";
    } else if (nae == "INJECTION_LOOP_ITER") {
      json& jj = getDef("InjectionPoints", Args->getUnexpArgument(0));
      json& stages = getOrCreate(jj, "stages");
      json& thisStage =
          getOrCreate(stages, pp.getSpelling(*Args->getUnexpArgument(1)));
      thisStage["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_LOOP_END") {
      json& jj = getDef("InjectionPoints", Args->getUnexpArgument(0));
      json& stages = getOrCreate(jj, "stages");
      json& thisStage = getOrCreate(stages, "End");
      thisStage["docs"] = getDocs(Range);
    }
  }

  void MacroDefined(const Token& MacroNameTok,
                    const MacroDirective* MD) override {
    if (active) return;
    std::string nae = pp.getSpelling(MacroNameTok);
    if (nae == "PACKAGENAME") {
      std::string declares = "";
      for (const Token& it :
           MD->getDefinition().getDirective()->getMacroInfo()->tokens()) {
        declares += pp.getSpelling(it);
      }
      thisJson["packageName"] = declares;
      // Add the comment handler now to minimize time spent handling comments
      // in files that do not even include the VnV.h file.
      if (packName.empty() ||
          VnV::StringUtils::trim_copy(declares) == packName) {
        pp.addCommentHandler(this);
        active = true;
      } else {
        // TODO -- Technically, we can abort here. Not sure if possible using
        // the pp callbacks API.
      }
    }
  }
};

class PreProcessVnV : public PreprocessorFrontendAction {
 public:
  json& mainJson;
  json subJson;
  std::string filename;
  std::string packName;
  std::set<std::string> includes;
  PreProcessVnV(json& m, std::string packageName)
      : mainJson(m), packName(packageName) {}
  ~PreProcessVnV() {}

  void ExecuteAction() {
    Preprocessor& PP = getCompilerInstance().getPreprocessor();
    SourceManager& SRC = PP.getSourceManager();

    filename = SRC.getFileEntryForID(SRC.getMainFileID())->getName().str();
    std::time_t lastMod =
        SRC.getFileManager().getFileRef(filename)->getModificationTime();
    subJson = json::object();
    PP.addPPCallbacks(
        std::make_unique<PreprocessCallback>(packName, subJson, includes, PP));
    PP.IgnorePragmas();

    Token Tok;
    // Start parsing the specified input file.
    PP.EnterMainSourceFile();
    do {
      PP.Lex(Tok);
    } while (Tok.isNot(tok::eof));

    mainJson[filename]["data"] = subJson;
    json j = json::array();
    for (auto& it : includes) {
      j.push_back(it);
    }
    mainJson[filename]["includes"] = j;
    includes.clear();
  }
};

class VnVPackageFinderFrontendActionFactory
    : public tooling::FrontendActionFactory {
 public:
  VnVPackageFinderFrontendActionFactory(json& processed, std::string pname)
      : packageName(pname), mainJson(processed) {}

  std::unique_ptr<FrontendAction> create() override {
    return std::make_unique<PreProcessVnV>(mainJson, packageName);
  }

 private:
  std::unique_ptr<PreProcessVnV> ptr;
  json& mainJson;
  std::string packageName;
};

class VnVFinder : public MatchFinder {
 public:
  VnVPrinter Printer;
  VnVFinder() {
    StatementMatcher functionMatcher =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("_VnV_injectionPoint"))))
            .bind("callsite");
    StatementMatcher functionMatcher1 =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("_VnV_injectionPoint_begin"))))
            .bind("callsite_begin");
    StatementMatcher functionMatcher2 =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("_VnV_injectionPoint_loop"))))
            .bind("callsite_iter");
    StatementMatcher functionMatcher3 =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("_VnV_injectionPoint_end"))))
            .bind("callsite_end");

    // Cpp Matchers
    StatementMatcher functionMatcherC =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("VnV::CppInjection::BeginPack"))))
            .bind("cpp_callsite");
    StatementMatcher functionMatcher1C =
        callExpr(
            hasAncestor(functionDecl().bind("function")),
            callee(functionDecl(hasName("VnV::CppInjection::BeginLoopPack"))))
            .bind("cpp_callsite_begin");
    StatementMatcher functionMatcher2C =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("VnV::CppInjection::IterLoop"))))
            .bind("callsite_iter");
    StatementMatcher functionMatcher3C =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("VnV::CppInjection::EndLoop"))))
            .bind("callsite_end");

    addMatcher(functionMatcherC, &Printer);
    addMatcher(functionMatcher1C, &Printer);
    addMatcher(functionMatcher2C, &Printer);
    addMatcher(functionMatcher3C, &Printer);
    addMatcher(functionMatcher, &Printer);
    addMatcher(functionMatcher1, &Printer);
    addMatcher(functionMatcher2, &Printer);
    addMatcher(functionMatcher3, &Printer);
  }
  json& get() { return Printer.get(); }
};

}  // namespace

void addAll(json& to, json& from) {
  for (auto it : from.items()) {
    to[it.key()] = it.value();
  }
}

void writeFileAndCache(json& cacheInfo, std::string outputFileName,
                       std::string cacheFile, std::string packageName,
                       bool writeFile) {
  // HEREEEEEE
  // Convert from the file->Objects cache data structure to a ObjectType ->
  // format.
  std::hash<std::string> hasher;
  if (writeFile) {
    llvm::outs() << "Writing VnV Registration File\n";
    json finalJson = json::object();
    for (auto it : cacheInfo["data"].items()) {
      if (!it.value().contains("packageName")) continue;

      for (std::string type :
           {"InjectionPoints", "SubPackages", "LogLevels", "Tests", "Engines",
            "Comms", "Reducers", "DataTypes", "Serializers", "Transforms",
            "UnitTests"}) {
        addAll(getOrCreate(finalJson, type), getOrCreate(it.value(), type));
      }
      if (it.value().contains("Options")) {
        std::string pname = it.value()["packageName"].get<std::string>();
        getOrCreate(finalJson, "Options")[pname] = it.value()["Options"];
      }
      if (it.value().contains("Communicator")) {
        std::string pname = it.value()["packageName"].get<std::string>();
        getOrCreate(finalJson, "Communicator")[pname] =
            it.value()["Communicator"];
      }
    }

    // Generate the registration code using the given json.
    RegistrationWriter r(finalJson, packageName);

    // Write the registraiton file for packageName_
    std::ofstream oss(outputFileName);
    oss << "/// This file was automatically generated using the VnV-Matcher "
           "executable. \n"
           "/// The matcher allows for automatic registration of all VnV "
           "plugins and injection \n"
           "/// points. Building the matcher requires Clang. If Clang is not "
           "available on this machine,\n"
           "/// Registration code should be written manually. \n"
           "/// \n\n";

    oss << r.printPackage(packageName);
    oss.close();
    } else {
      llvm::outs() << "VnV Registration File Unchanged\n";
    }

  llvm::outs() << "Writing VnV Registration Cache\n";
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

  // Write the cache.
  std::ofstream cacheStream(cacheFile);

  // Clean up the cache. Basically, remove any files
  time_t now;
  time(&now);
  cacheInfo[LAST_RUN_TIME] = timeToISOString(&now);
  cacheInfo[LAST_FILE_NAME] = outputFileName;
  cacheStream << cacheInfo.dump();
  cacheStream.close();
}

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


  json& cacheMap = getOrCreate(cacheInfo, "map");
  json& cacheFiles = getOrCreate(cacheInfo, "files");
  json& cacheData = getOrCreate(cacheInfo, "data");

  std::hash<std::string> hasher;
  std::size_t cacheDataHash = hasher(cacheData.dump());

  bool hasCache = (cacheInfo.contains(LAST_RUN_TIME));
  std::vector<std::string> modFiles;
  if (hasCache) {
    std::vector<std::string> files =
        OptionsParser.getCompilations().getAllFiles();
    std::string lastRunTime = cacheInfo[LAST_RUN_TIME].get<std::string>();

    std::map<std::string, bool> fModMap;

    for (auto it : cacheMap.items()) {
      fModMap[it.key()] =
          (timeForFile(it.value().get<std::string>()) > lastRunTime);
    }

    for (auto& it : files) {  // all files to be compiled. (strings)
      if (cacheFiles.contains(it)) {
        for (auto f : cacheFiles[it].items()) {
          // list of ids included in this file.
          std::string id = f.value().get<std::string>();
          if (fModMap[id]) {
            modFiles.push_back(it);
            break;
          }
        }
      } else {
        modFiles.push_back(it);  // New File not prev in cache.
      }
    }
    // TODO CHECK IF FILE WAS MODIFIED MANUALLY .
    if (modFiles.size() == 0) {
      if (outputFileName != cacheInfo[LAST_FILE_NAME] ||
          lastRunTime < timeForFile(outputFileName)) {
        writeFileAndCache(cacheInfo, outputFileName, cacheFile_, packageName_,true);
        llvm::outs() << "No Changes Detected But FileName Changed -->Copying from Cache.\n";
        return 0;
      }
      llvm::outs() << "No Changes Detected.\n";
      return 0;
    }
  } else {
    modFiles = OptionsParser.getCompilations().getAllFiles();
  }
  llvm::outs() << "Changes Detected -> Regenerating VnV Registration File \n";
  // Generate the main VnV Declares object.
  ClangTool VnVTool1(OptionsParser.getCompilations(), modFiles);
  json vnvDeclares1 = json::object();
  VnVPackageFinderFrontendActionFactory factory1(vnvDeclares1, packageName_);
  VnVTool1.run(&factory1);

  // Get a list of files that contain Injection points with the current package
  // Name.
  std::vector<std::string> injectionFiles;
  for (auto it : vnvDeclares1.items()) {
    // update the cache for this file
    json incs = json::array();
    for (auto itt : it.value()["includes"].items()) {
      std::string s = std::to_string(hasher(itt.value()));  // Hash the filename
      if (!cacheMap.contains(s)) {
        cacheMap[s] = itt.value();
      }
      incs.push_back(s);
    }
    cacheFiles[it.key()] = incs;

    json& data = it.value()["data"];

    if ((packageName_.empty() ||
         (data.contains("packageName") &&
          data["packageName"].get<std::string>() == packageName_))) {
      if (data.contains("InjectionPoints") &&
          data["InjectionPoints"].size() > 0) {
        injectionFiles.push_back(it.key());
      }
      cacheData[it.key()] = data;
    }
  }

  // Search the AST to extract information about injection points. In
  // particular,
  // we search the AST to extract the parameter types.
  VnVFinder Finder;
  ClangTool Tool(OptionsParser.getCompilations(), injectionFiles);
  int x = Tool.run(newFrontendActionFactory(&Finder).get());

  // Add the injection point data to the cacheData object.
  for (auto fil : Finder.get().items()) {
    std::string fname = fil.key();
    json& cdata = getOrCreate(cacheData, fname);
    json& injectData = getOrCreate(cdata, "InjectionPoints");
    for (auto it : fil.value().items()) {
      json& ip = getOrCreate(injectData,
                             it.key());  // the data for this injection point.
      json& val = it.value();

      ip["parameters"] = (val.contains("parameters") ? it.value()["parameters"]
                                                     : json::array());
      json& stages = ip["stages"];
      for (auto& stage : val["stages"].items()) {
        stages[stage.key()]["info"] = stage.value();
      }
    }
  }

  // Write the file and cache. Always update the cache -- to get the updated
  // times, but only write the file if
  // the filename has changed or the actual data has changed.
  bool writeFile = ((cacheInfo[LAST_FILE_NAME] != outputFileName) ||
                    hasher(cacheData.dump()) != cacheDataHash);

  writeFileAndCache(cacheInfo, outputFileName, cacheFile_, packageName_,
                    writeFile);
  return 0;
}
