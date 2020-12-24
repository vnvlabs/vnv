#include "base/Utilities.h"

#include <clang/AST/RawCommentList.h>
#include <clang/Basic/CommentOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Tooling.h>

#include <iostream>
#include <set>

using namespace clang;
using namespace clang::tooling;
using nlohmann::json;

namespace {

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

}  // namespace

using nlohmann::json;
using namespace clang;

class PreprocessCallback : public PPCallbacks, CommentHandler {
 public:
  Preprocessor& pp;
  json& thisJson;
  CommentOptions commentOptions;
  std::set<std::string>& modTime;
  std::vector<std::unique_ptr<RawComment>> currComment;
  std::vector<SourceLocation> currentLoc;
  std::string packName;

  json* lastTestJson = nullptr;

  std::string lastUnitTestPackage;

  bool active = false;
  bool realFile = false;

  PreprocessCallback(std::string packageName, json& j,
                     std::set<std::string>& includes, Preprocessor& PP)
      : PPCallbacks(), pp(PP), thisJson(j), modTime(includes) {
    commentOptions.ParseAllComments = false;
    packName = packageName;
    // PP.addCommentHandler(this);
  }

  std::string getCommentFor(DiagnosticsEngine& eng, SourceManager& SM,
                            SourceLocation loc) {
    SourceLocation macroSrcLoc = SM.getExpansionLoc(loc);
    std::pair<FileID, unsigned> macroLoc = SM.getDecomposedLoc(macroSrcLoc);
    while (currComment.size() > 0) {
      std::pair<FileID, unsigned> lastCommLoc =
          SM.getDecomposedLoc(currentLoc.back());
      // Question does not make sense if locations are in different files.
      if (macroLoc.first != lastCommLoc.first) {
        return "";
      }

      if (lastCommLoc.second > macroLoc.second) {
        // Catch cases where there were comments in the lambda function.
        currComment.pop_back();
        currentLoc.pop_back();
      } else if (onlyWhitespaceBetween(SM, currentLoc.back(),
                                       SM.getExpansionLoc(loc), 1000)) {
        std::string s = currComment.back()->getFormattedText(SM, eng);
        currComment.clear();  // reset(nullptr);
        currentLoc.clear();
        return s;
      } else {
        currComment.clear();  // reset(nullptr);
        currentLoc.clear();
      }
    }
    return "";
  }

  bool HandleComment(Preprocessor& PP, SourceRange Comment) override {
    SourceManager& SM = PP.getSourceManager();
    RawComment* c = new RawComment(SM, Comment, commentOptions, false);
    if (c->isDocumentation()) {
      currentLoc.push_back(Comment.getEnd());
      currComment.emplace_back(c);
    } else {
      delete c;
    }
    return false;
  }

  json& getDef(std::string type, std::string packageName, std::string nstr) {
    json& j = VnV::JsonUtilities::getOrCreate(thisJson, type);
    json& jj = VnV::JsonUtilities::getOrCreate(j, packageName + ":" + nstr);
    jj["name"] = nstr;
    jj["packageName"] = packageName;
    return jj;
  }

  std::string getDocs(SourceRange Range) {
    return getCommentFor(pp.getDiagnostics(), pp.getSourceManager(),
                         Range.getBegin());
  }

  std::string getPackageName(const MacroArgs* Args, int c) {
    std::string s = "";
    for (auto it : const_cast<MacroArgs*>(Args)->getPreExpArgument(c, pp)) {
      s += pp.getSpelling(it);
    }
    return s;
  }

  void FileChanged(SourceLocation Loc, FileChangeReason Reason,
                   SrcMgr::CharacteristicKind FileType,
                   FileID PrevFID) override {
    std::string fname = pp.getSourceManager().getFilename(Loc).str();
    if (!fname.empty()) {
      auto f = pp.getSourceManager().getFileManager().getFile(fname);
      if (f && active) {
        modTime.insert(fname);  // = f->getModificationTime();
      }
      realFile = true;
    } else {
      realFile = false;
    }
    currentLoc.clear();
    currComment.clear();
  }

  void MacroExpands(const Token& MacroNameTok, const MacroDefinition& MD,
                    SourceRange Range, const MacroArgs* Args) override {
    if (!active) return;

    std::string nae = pp.getSpelling(MacroNameTok);

    // TODOO CATCH THE TEST ASSERT EQUALS CALLS FOR GETTING DOCS FOR INDIVIDUAL
    // UNIT TESTS

    if ((nae == "TEST_ASSERT_EQUALS" || nae == "TEST_ASSERT_NOT_EQUALS") &&
        lastTestJson != nullptr) {
      std::string name = VnV::StringUtils::trim_copy(
          pp.getSpelling(*Args->getUnexpArgument(0)));
      if (name.size() < 3) {
        std::cout << "WARNING VNV TEST INVALID NAME" << std::endl;
      }

      if (name[0] == '\"' && name[name.size() - 1] == '\"') {
        name = name.substr(1, name.size() - 2);
      }

      json& j = (*lastTestJson)["tests"];
      std::string docs = getDocs(Range);
      auto t = j.find(name);
      if (t == j.end()) {
        j[name] = docs;
      } else {
        j[name] = t.value().get<std::string>() + docs;
      }
      return;
    }

    if (nae.substr(0, 10).compare("INJECTION_") != 0) return;

    // Reset the lastTestJson object as we have a new Injection macro.
    lastTestJson = nullptr;
    if (nae == "INJECTION_TEST_RS") {
      json& jj =
          getDef("Tests", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
      jj["parameters"] = parseArgs(pp, Args->getUnexpArgument(4));
    } else if (nae == "INJECTION_ITERATOR_RS") {
        json& jj =
            getDef("Iterators", getPackageName(Args, 0), getPackageName(Args, 1));
        jj["docs"] = getDocs(Range);
        jj["parameters"] = parseArgs(pp, Args->getUnexpArgument(4));
    } else if (nae == "INJECTION_SERIALIZER_R") {
      json& jj = getDef("Serializers", getPackageName(Args, 0),
                        getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
      jj["type"] = stringArgs(pp, Args->getUnexpArgument(3), false);
    } else if (nae == "INJECTION_TRANSFORM_R") {
      json& jj = getDef("Transforms", getPackageName(Args, 0),
                        getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
      jj["to"] = stringArgs(pp, Args->getUnexpArgument(3), false);
      jj["from"] = stringArgs(pp, Args->getUnexpArgument(4), false);
    } else if (nae == "INJECTION_OPTIONS") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Options");
      std::string pname = getPackageName(Args, 0);
      jj[pname] = getDocs(Range);
    } else if (nae == "INJECTION_LOGLEVEL") {
      json& jj =
          getDef("LogLevels", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["color"] = pp.getSpelling(*Args->getUnexpArgument(2));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_SUBPACKAGE") {
      json& jj = getDef("SubPackages", getPackageName(Args, 0),
                        getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_UNITTEST_R" ||
               nae == "INJECTION_UNITTEST_RAW") {
      json& jj =
          getDef("UnitTests", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
      jj["tests"] = json::object();
      lastTestJson = &jj;

    } else if (nae == "INJECTION_ENGINE") {
      json& jj =
          getDef("Engines", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_DATATYPE") {
      json& jj =
          getDef("DataTypes", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_REDUCER") {
      json& jj =
          getDef("Reducers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_COMM") {
      json& jj =
          getDef("Comms", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_INITIALIZE") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Introduction");
      jj[getPackageName(Args, 0)] = getDocs(Range);
    } else if (nae == "INJECTION_FINALIZE") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Conclusion");
      jj[getPackageName(Args, 0)] = getDocs(Range);
    } else if (nae == "INJECTION_PACKAGEDOCS") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Package");
      std::string pname = pp.getSpelling(*Args->getUnexpArgument(0));
      jj[pname] = getDocs(Range);
    } else if (nae == "INJECTION_COMMUNICATOR" ||
               nae == "INJECTION_EXECUTABLE") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Communicator");
      json& jk = VnV::JsonUtilities::getOrCreate(jj, getPackageName(Args, 0));
      jk["docs"] = getDocs(Range);
      jk["package"] = getPackageName(Args, 1);
      jk["name"] = getPackageName(Args, 2);
    } else if (nae == "INJECTION_POINT_C" || nae == "INJECTION_LOOP_BEGIN_C") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 0),
                        getPackageName(Args, 2));
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "Begin");
      jj["docs"] = getDocs(Range);
      thisStage["docs"] = "";
    } else if (nae == "INJECTION_ITERATION_C" || nae == "INJECTION_ITERATION") {
        json& jj = getDef("InjectionPoints", getPackageName(Args, 0),
                          getPackageName(Args, 2));
        json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
        json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "Begin");
        jj["docs"] = getDocs(Range);
        jj["iterator"] = true;
        thisStage["docs"] = "";
      } else if (nae == "INJECTION_LOOP_ITER") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 0),
                        getPackageName(Args, 1));
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage =
          VnV::JsonUtilities::getOrCreate(stages, getPackageName(Args, 2));
      thisStage["docs"] = getDocs(Range);
    } else if (nae == "INJECTION_LOOP_END") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 0),
                        getPackageName(Args, 1));
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "End");
      thisStage["docs"] = getDocs(Range);
    }
  }


  void MacroDefined(const Token& MacroNameTok,
                    const MacroDirective* MD) override {
    if (active) return;
    std::string nae = pp.getSpelling(MacroNameTok);
    if (nae == "VNV_INCLUDED") {
      pp.addCommentHandler(this);
      active = true;
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
  std::string packageName;
  json& mainJson;
};

json runPreprocessor(CompilationDatabase& comps,
                     std::vector<std::string>& files,
                     std::string packageName_) {
  // Generate the main VnV Declares object.
  ClangTool VnVTool(comps, files);
  json j = json::object();
  VnVPackageFinderFrontendActionFactory factory(j, packageName_);
  VnVTool.run(&factory);
  return j;
}
