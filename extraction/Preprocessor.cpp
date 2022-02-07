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

#include "base/Utilities.h"

using namespace clang;
using namespace clang::tooling;
using nlohmann::json;

namespace {

class ProcessedComment {
 public:
  std::string templ = "";
  std::string title = "";
  std::string description = "";
  std::string instructions = "";
  std::map<std::string, std::string> params;
  ProcessedComment(){};
  ProcessedComment(std::string templ_, std::string title_, std::string desc, std::string inst,
                   const std::map<std::string, std::string>& m)
      : templ(templ_), title(title_), description(desc), instructions(inst), params(m) {}

  json toJson() {
    json j = json ::object();
    j["template"] = templ;
    j["title"] = title;
    j["description"] = description;
    j["instructions"] = instructions;
    j["params"] = params;
    return j;
  }
};

ProcessedComment processComment(std::string comment) {
  /**
   * @title This is the title
   * @description This is the description and it can be single lines.
   * @instructions These are the instructions.
   * @param x sdfsdfdsf
   *
   * sdfsdfs
   * sdfsdfsdfsdf
   *
   */
  ProcessedComment c;
  std::vector<std::string> result;
  VnV::StringUtils::StringSplit(comment, "\n", result, true);
  std::ostringstream oss;

  std::string* curr = nullptr;
  for (auto it : result) {
    if (c.title.empty()) {
      auto t = it.find("@title");
      if (t != std::string::npos) {
        c.title = VnV::StringUtils::trim_copy(it.substr(t + 6));
        curr = &(c.title);
        continue;
      }
    }
    if (c.description.empty()) {
      auto t = it.find("@description");
      if (t != std::string::npos) {
        c.description = VnV::StringUtils::trim_copy(it.substr(t + 12));
        curr = &(c.description);
        continue;
      }
    }
    if (c.instructions.empty()) {
      auto t = it.find("@instructions");
      if (t != std::string::npos) {
        c.instructions = VnV::StringUtils::trim_copy(it.substr(t + 13));
        curr = &(c.instructions);
        continue;
      }
    }
    auto t = it.find("@param");
    if (t != std::string::npos) {
      auto subs = VnV::StringUtils::ltrim_copy(it.substr(t + 6));
      auto tt = subs.find_first_of(" ");
      if (tt != std::string::npos) {
        c.params[subs.substr(0, tt)] = subs.substr(tt + 1);
        curr = &(c.params[subs.substr(0, tt)]);
      } else {
        c.params[subs] = "No description provided";
        curr = &(c.params[subs.substr(0, tt)]);
      }
      continue;
    }
    if (curr != nullptr) {
      auto s = VnV::StringUtils::trim_copy(it);
      if (s.length() > 0) {
        (*curr) += "\n" + it;
        continue;
      }
    }
    curr = nullptr;
    oss << it << "\n";
  }
  c.templ = oss.str();
  return c;
}

// Copied shamelessly from an anonomous namespace in the Clang source code in
// RawCommentList.cpp
static bool onlyWhitespaceBetween(SourceManager& SM, SourceLocation Loc1, SourceLocation Loc2,
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
      if (I + 1 != Loc2Info.second && (Buffer[I + 1] == '\n' || Buffer[I + 1] == '\r') && Buffer[I] != Buffer[I + 1])
        ++I;
      break;
    }
  }

  return true;
}

std::string stringArgs(Preprocessor& pp, const Token* token, bool spaceBeforeLast) {
  std::string s = "";
  while (token->isNot(tok::eof)) {
    if (spaceBeforeLast && (pp.getSpelling(*(token + 1)) == "," || (token + 1)->is(tok::eof))) s += " ";
    s += pp.getSpelling(*token);
    token++;
  }
  return s;
}

}  // namespace

using nlohmann::json;
using namespace clang;

class PreprocessCallback : public PPCallbacks, CommentHandler {
 public:
  Preprocessor& pp;
  json& thisJson;

  json docOverrides;

  CommentOptions commentOptions;
  std::set<std::string>& modTime;
  std::vector<std::unique_ptr<RawComment>> currComment;
  std::vector<SourceLocation> currentLoc;

  json* lastTestJson = nullptr;
  json* lastWorkflowJson = nullptr;

  std::string lastUnitTestPackage;

  bool active = false;
  bool realFile = false;

  PreprocessCallback(json& j, std::set<std::string>& includes, Preprocessor& PP)
      : PPCallbacks(), pp(PP), thisJson(j), modTime(includes) {
    commentOptions.ParseAllComments = false;
    // PP.addCommentHandler(this);
  }

  void override_comment(SourceRange range, std::string packageName, std::string name, std::string type,
                        int importannce) {
    json& docOverrides = VnV::JsonUtilities::getOrCreate(thisJson, "doc_overrides");
    json& j = VnV::JsonUtilities::getOrCreate(docOverrides, type);
    json& jj = VnV::JsonUtilities::getOrCreate(j, packageName + ":" + name);

    if (jj.contains("imp") && jj["imp"].get<int>() > importannce) {
      return;
    }
    jj["imp"] = importannce;
    jj["docs"] = getDocs(range).toJson();
  }

  ProcessedComment getCommentFor(DiagnosticsEngine& eng, SourceManager& SM, SourceLocation loc) {
    SourceLocation macroSrcLoc = SM.getExpansionLoc(loc);
    std::pair<FileID, unsigned> macroLoc = SM.getDecomposedLoc(macroSrcLoc);
    while (currComment.size() > 0) {
      std::pair<FileID, unsigned> lastCommLoc = SM.getDecomposedLoc(currentLoc.back());
      // Question does not make sense if locations are in different files.
      if (macroLoc.first != lastCommLoc.first) {
        return processComment("");
      }

      if (lastCommLoc.second > macroLoc.second) {
        // Catch cases where there were comments in the lambda function.
        currComment.pop_back();
        currentLoc.pop_back();
      } else if (onlyWhitespaceBetween(SM, currentLoc.back(), SM.getExpansionLoc(loc), 1000)) {
        std::string s = currComment.back()->getFormattedText(SM, eng);
        currComment.clear();  // reset(nullptr);
        currentLoc.clear();
        return processComment(s);
      } else {
        currComment.clear();  // reset(nullptr);
        currentLoc.clear();
      }
    }
    return processComment("");
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

  ProcessedComment getDocs(SourceRange Range) {
    return getCommentFor(pp.getDiagnostics(), pp.getSourceManager(), Range.getBegin());
  }

  std::string getPackageName(const MacroArgs* Args, int c, bool removeQuotes = false) {
    std::string s = "";
    for (auto it : const_cast<MacroArgs*>(Args)->getPreExpArgument(c, pp)) {
      s += pp.getSpelling(it);
    }
    return removeQuotes ? s.substr(1, s.size() - 2) : s;
  }

  void FileChanged(SourceLocation Loc, FileChangeReason Reason, SrcMgr::CharacteristicKind FileType,
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

  void MacroExpands(const Token& MacroNameTok, const MacroDefinition& MD, SourceRange Range,
                    const MacroArgs* Args) override {
    if (!active) return;

    std::string nae = pp.getSpelling(MacroNameTok);

    // TODOO CATCH THE TEST ASSERT EQUALS CALLS FOR GETTING DOCS FOR INDIVIDUAL
    // UNIT TESTS
     
  

    if ((nae == "TEST_ASSERT_EQUALS" || nae == "TEST_ASSERT_NOT_EQUALS") && lastTestJson != nullptr) {
      std::string name = VnV::StringUtils::trim_copy(pp.getSpelling(*Args->getUnexpArgument(0)));
      if (name.size() < 3) {
        std::cout << "WARNING VNV TEST INVALID NAME" << std::endl;
      }

      if (name[0] == '\"' && name[name.size() - 1] == '\"') {
        name = name.substr(1, name.size() - 2);
      }

      json& j = (*lastTestJson)["tests"];
      std::string docs = getDocs(Range).templ;
      auto t = j.find(name);
      if (t == j.end()) {
        j[name] = docs;
      } else {
        j[name] = t.value().get<std::string>() + docs;
      }
      return;
    } else if (nae == "INJECTION_CREATE_JOB" &&  lastWorkflowJson != nullptr )  {

         std::string name = VnV::StringUtils::trim_copy(pp.getSpelling(*Args->getUnexpArgument(1)));
         if (name[0] == '\"' && name[name.size() - 1] == '\"') {
             name = name.substr(1, name.size() - 2);
         }
         json &j = (*lastWorkflowJson)["jobs"];
         j[name] = getDocs(Range).toJson();
         return;
    }


    if (nae.substr(0, 10).compare("INJECTION_") != 0) return;

    // Reset the lastTestJson object as we have a new Injection macro.
    lastTestJson = nullptr;
    lastWorkflowJson = nullptr;
    if (nae == "INJECTION_COMMENT") {
      std::string pname = getPackageName(Args, 0);
      std::string name = getPackageName(Args, 1);
      std::string type = getPackageName(Args, 2);
      int import = std::atoi(getPackageName(Args, 3).c_str());
      override_comment(Range, pname, name, type, import);

    } else if (nae == "INJECTION_TEST_RS") {
      json& jj = getDef("Tests", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_ITERATOR_RS") {
      json& jj = getDef("Iterators", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_PLUG_RS") {
      json& jj = getDef("Plugs", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_SERIALIZER_R") {
      json& jj = getDef("Serializers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
      jj["type"] = stringArgs(pp, Args->getUnexpArgument(3), false);
    } else if (nae == "INJECTION_TRANSFORM_R") {
      json& jj = getDef("Transforms", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
      jj["to"] = stringArgs(pp, Args->getUnexpArgument(3), false);
      jj["from"] = stringArgs(pp, Args->getUnexpArgument(4), false);
      jj["alias"] = false;
    } else if (nae == "INJECTION_ALIAS") {
      json& jj = getDef("Transforms", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
      jj["to"] = stringArgs(pp, Args->getUnexpArgument(2), false);
      jj["from"] = stringArgs(pp, Args->getUnexpArgument(3), false);
      jj["alias"] = true;
    } else if (nae == "INJECTION_OPTIONS") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Options");
      std::string pname = getPackageName(Args, 0);
      jj[pname] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_INPUT_FILE_") {
      json& jj = getDef("Files", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_OUTPUT_FILE_") {
      json& jj = getDef("Files", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_LOGLEVEL") {
      json& jj = getDef("LogLevels", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["color"] = pp.getSpelling(*Args->getUnexpArgument(2));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_SUBPACKAGE") {
      json& jj = getDef("SubPackages", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_UNITTEST_R" || nae == "INJECTION_UNITTEST_RAW") {
      json& jj = getDef("UnitTests", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
      jj["tests"] = json::object();
      lastTestJson = &jj;
    } else if (nae == "INJECTION_ACTION") {
      json& jj = getDef("Actions", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_ENGINE") {
      json& jj = getDef("Engines", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_ENGINE_READER") {
      json& jj = getDef("EngineReaders", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_DATATYPE") {
      json& jj = getDef("DataTypes", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_REDUCER") {
      json& jj = getDef("Reducers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_SCHEDULER") {
      json& jj = getDef("Schedulers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_VALIDATOR") {
      json& jj = getDef("Validators", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_JOBCREATOR") {
      json& jj = getDef("JobCreators", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["jobs"] = json::object();
      jj["docs"] = getDocs(Range).toJson();
      lastWorkflowJson = &jj;
    } else if (nae == "INJECTION_SCRIPTGENERATOR") {
      json& jj = getDef("ScriptGenerators", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    }  else if (nae == "INJECTION_SAMPLER_RS") {
      json& jj = getDef("Samplers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_WALKER_S") {
      json& jj = getDef("Walkers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_COMM") {
      json& jj = getDef("Comms", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_INITIALIZE" || nae == "INJECTION_INITIALIZE_RAW") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Introduction");
      jj[getPackageName(Args, 0)] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_FINALIZE") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Conclusion");
      jj[getPackageName(Args, 0)] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_PACKAGEDOCS") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Package");
      std::string pname = pp.getSpelling(*Args->getUnexpArgument(0));
      jj[pname] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_POINT_C" || nae == "INJECTION_LOOP_BEGIN_C") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 0, true), getPackageName(Args, 2, true));

      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "Begin");
      jj["docs"] = getDocs(Range).toJson();
      thisStage["docs"] = ProcessedComment().toJson();
      if (nae == "INJECTION_LOOP_BEGIN_C") {
        jj["loop"] = true;
      }

    } else if (nae == "INJECTION_ITERATION_C") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 1, true), getPackageName(Args, 3, true));
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "Begin");
      jj["docs"] = getDocs(Range).toJson();
      jj["iterator"] = true;
      thisStage["docs"] = ProcessedComment().toJson();

    } else if (nae == "INJECTION_FUNCTION_PLUG_C") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 1, true), getPackageName(Args, 3, true));
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "Begin");
      jj["docs"] = getDocs(Range).toJson();
      jj["plug"] = true;
      thisStage["docs"] = ProcessedComment().toJson();
     
    } else if (nae == "INJECTION_LOOP_ITER") {
      
      //json& jj = getDef("InjectionPoints", getPackageName(Args, 0, true), getPackageName(Args, 1, true));
      //json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      //json& thisStage = VnV::JsonUtilities::getOrCreate(stages, getPackageName(Args, 2, true));
      //thisStage["docs"] = getDocs(Range).toJson();
    
    } else if (nae == "INJECTION_LOOP_END") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 0, true), getPackageName(Args, 1, true));
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "End");
      thisStage["docs"] = getDocs(Range).toJson();
    }
  }

  void MacroDefined(const Token& MacroNameTok, const MacroDirective* MD) override {
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
  std::set<std::string> includes;
  PreProcessVnV(json& m) : mainJson(m) {}
  ~PreProcessVnV() {}

  bool BeginInvocation(CompilerInstance &CI) override {
      //Weird bug in Clang -- When we change directories, the file manager seems to end up in  
      //the wrong directory. Resetting the file manager here means a new one will be created. This 
      //new one is created in the correct directory, so we can continue. 
      CI.setFileManager(nullptr);
      CI.setSourceManager(nullptr);
      return true;
  }

  void ExecuteAction() {
    
    Preprocessor& PP = getCompilerInstance().getPreprocessor();
    SourceManager& SRC = PP.getSourceManager();

    filename = SRC.getFileEntryForID(SRC.getMainFileID())->tryGetRealPathName().str();
    subJson = json::object();
    PP.addPPCallbacks(std::make_unique<PreprocessCallback>(subJson, includes, PP));
    PP.IgnorePragmas();
    
    std::cout << "Processing " << filename << "  " << &PP.getHeaderSearchInfo() <<  std::endl; 

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

class VnVPackageFinderFrontendActionFactory : public tooling::FrontendActionFactory {
 public:
  VnVPackageFinderFrontendActionFactory(json& processed) : mainJson(processed) {}

  std::unique_ptr<FrontendAction> create() override { return std::make_unique<PreProcessVnV>(mainJson); }

  void finalize() {
    if (mainJson.contains("doc_overrides")) {
      nlohmann::json overrides = mainJson["doc_overrides"];
      for (auto type : overrides.items()) {
        if (mainJson.contains(type.key())) {
          json tjson = mainJson[type.key()];
          for (auto& n : type.value().items()) {
            if (tjson.contains(n.key())) {
              tjson[n.key()]["docs"] = n.value()["docs"];
            } else {
              std::cout << "Error -- No " << type.key() << " named " << n.key() << std::endl;
            }
          }
        } else {
          std::cout << "Error -- No type named " << type.key() << std::endl;
        }
      }
      mainJson.erase("doc_overrides");
    }
  }

 private:
  std::unique_ptr<PreProcessVnV> ptr;
  json& mainJson;
};

json runPreprocessor(CompilationDatabase& comps, std::set<std::string>& files) {
  // Generate the main VnV Declares object.
  std::vector<std::string> files_vec(files.begin(), files.end());
  for (auto &it : files_vec) {
    std::cout << it << std::endl;
  }
  
  ClangTool VnVTool(comps, files_vec);
  json j = json::object();
  VnVPackageFinderFrontendActionFactory factory(j);
  VnVTool.setRestoreWorkingDir(true);
  VnVTool.run(&factory);
  factory.finalize();
  return j;
}
