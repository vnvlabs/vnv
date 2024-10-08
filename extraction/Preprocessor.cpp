﻿#include <clang/AST/RawCommentList.h>
#include <clang/Basic/CommentOptions.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Tooling/Tooling.h>

#include <iostream>
#include <regex>
#include <set>

#include "shared/Utilities.h"

using namespace clang;
using namespace clang::tooling;
using nlohmann::json;

namespace {

/**
 * comment that implements a VnV macro
 *   stores information as json
 */
class ProcessedComment {
 public:
  // VnV macro comment's anatomy before being interpreted by pre-processor
  //  this anatomy is defined in the comment above ProcessedComment::processComment(std::string comment)
  std::string templ = "";
  std::string title = "";
  std::string treeTitle = "";
  std::string description = "";
  std::string instructions = "";
  std::string configuration = "";
  std::map<std::string, std::string> params;
  ProcessedComment(){};

  // create a json file to store VnV macro comment anatomy
  json toJson() {
    json j = json ::object();
    j["template"] = templ;
    j["title"] = title;
    j["description"] = description;
    j["instructions"] = instructions;
    j["shortTitle"] = treeTitle;
    j["params"] = params;

    // if there's no string for configuration, then set configuration to the json object
    if (configuration.empty()) {
      j["configuration"] = json::object();
    } else {
      // otherwise, try to convert the configuration string into a json object
      try {
        j["configuration"] = json::parse(configuration);
      } catch (...) {
        // output error message if cannot convert the configuration string into a json
        std::cout << "Could not parse configuration -- Invalid Json" << std::endl;
        std::cout << j.dump(3);
        std::cout << "Ignoring invalid configuration options..." << std::endl;
      }
    }

    // return the json file that stores information about the processed VnV macro comment's anatomy
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
   * @configuration {
   *   "sdfsdf" : {
   *       "no-blank-lines-aloud" : ""
   *    }
   * }
   *
   * sdfsdfs
   * sdfsdfsdfsdf
   *
   */
  ProcessedComment c;
  // all of the processed comment's anatomy is stored in the result vector
  std::vector<std::string> result;
  VnV::StringUtils::StringSplit(comment, "\n", result, true);
  std::ostringstream oss;

  std::string* curr = nullptr;
  for (auto it : result) {
    // define the title anatomy if it hasn't been done already
    if (c.title.empty()) {
      // search for the title of the VnV macro comment
      auto t = it.find("@title");
      // if a title is found ...
      if (t != std::string::npos) {
        // set the processed comment's title to the foo in @title foo
        c.title = VnV::StringUtils::trim_copy(it.substr(t + 6));
        // move the current location pointer to the title
        curr = &(c.title);
        // move on to next type of comment anatomy
        continue;
      }
    }
    // SEE THE COMMENT FOR:
    //   if (c.title.empty()) {...}
    //     just consider the @description instead of the @title
    if (c.description.empty()) {
      auto t = it.find("@description");
      if (t != std::string::npos) {
        c.description = VnV::StringUtils::trim_copy(it.substr(t + 12));
        curr = &(c.description);
        continue;
      }
    }
    // SEE THE COMMENT FOR:
    //   if (c.title.empty()) {...}
    //     just consider the @instructions instead of the @title
    if (c.instructions.empty()) {
      auto t = it.find("@instructions");
      if (t != std::string::npos) {
        c.instructions = VnV::StringUtils::trim_copy(it.substr(t + 13));
        curr = &(c.instructions);
        continue;
      }
    }
    // SEE THE COMMENT FOR:
    //   if (c.title.empty()) {...}
    //     just consider the @shortTitle instead of the @title
    if (c.treeTitle.empty()) {
      auto t = it.find("@shortTitle");
      if (t != std::string::npos) {
        c.treeTitle = VnV::StringUtils::trim_copy(it.substr(t + 11));
        curr = &(c.treeTitle);
        continue;
      }
    }
    // SEE THE COMMENT FOR:
    //   if (c.title.empty()) {...}
    //     just consider the @configuration instead of the @title
    if (c.configuration.empty()) {
      auto t = it.find("@configuration");
      if (t != std::string::npos) {
        c.configuration = VnV::StringUtils::trim_copy(it.substr(t + 14));
        curr = &(c.configuration);
        continue;
      }
    }

    // search for the parameters of the VnV macro comment
    auto t = it.find("@param");
    // if @param is found ...
    if (t != std::string::npos) {
      // TODO doc
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
    // if VnV macro comment anatomy was previously found
    if (curr != nullptr) {
      // TODO doc
      auto s = VnV::StringUtils::trim_copy(it);
      if (s.length() > 0) {
        (*curr) += "\n" + it;
        continue;
      }
    }
    curr = nullptr;
    oss << it << "\n";
  }
  // set the template of the processed comment
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

/**
 * extract arguments of VnV macro Token
 */
std::string stringArgs(Preprocessor& pp, const Token* token, bool spaceBeforeLast) {
  // TODO doc
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
    
    json& docOverrides = VnV::JsonUtilities::getOrCreate(thisJson, "Overrides");
    json& jj = VnV::JsonUtilities::getOrCreate(docOverrides, packageName + ":" + name);

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

  /**
   * expand out a VnV macro Token to populate a json
   */
  void MacroExpands(const Token& MacroNameTok, const MacroDefinition& MD, SourceRange Range,
                    const MacroArgs* Args) override {
    // stop if the preprocessor is not turned on
    if (!active) return;

    // extract the name of the macro MacroNameTok Token object
    //  Token is a struct that is defined in injection/thrid-party/inja.hpp
    std::string nae = pp.getSpelling(MacroNameTok);

    // TODOO CATCH THE TEST ASSERT EQUALS CALLS FOR GETTING DOCS FOR INDIVIDUAL
    // UNIT TESTS

    // TODO doc
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
    } else if (nae == "INJECTION_CREATE_JOB" && lastWorkflowJson != nullptr) {
      // TODO doc
      std::string name = VnV::StringUtils::trim_copy(pp.getSpelling(*Args->getUnexpArgument(1)));
      if (name[0] == '\"' && name[name.size() - 1] == '\"') {
        name = name.substr(1, name.size() - 2);
      }
      json& j = (*lastWorkflowJson)["jobs"];
      j[name] = getDocs(Range).toJson();
      return;
    }

    // stop if nae's first 10 characters are not INJECTION_
    //  all VnV macros begin with INJECTION_
    if (nae.substr(0, 10).compare("INJECTION_") != 0) return;

    // Reset the lastTestJson object as we have a new Injection macro.
    lastTestJson = nullptr;
    lastWorkflowJson = nullptr;

    if (nae == "INJECTION_EXT_CALLBACK" ) {
      std::string pname = getPackageName(Args, 0);
      std::string name = getPackageName(Args, 1);
      override_comment(Range, pname, name, "InjectionPoints", 10);
    } else if (nae == "INJECTION_TEST_RS") {
      json& jj = getDef("Tests", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    }  else if (nae == "INJECTION_SERIALIZER_R") {
      json& jj = getDef("Serializers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
      jj["type"] = stringArgs(pp, Args->getUnexpArgument(3), false);
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
    } else if (nae == "INJECTION_WORKFLOW") {
      json& jj = getDef("Workflows", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["jobs"] = json::object();
      jj["docs"] = getDocs(Range).toJson();
      lastWorkflowJson = &jj;
    } else if (nae == "INJECTION_SCRIPTGENERATOR") {
      json& jj = getDef("ScriptGenerators", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_SAMPLER_RS") {
      json& jj = getDef("Samplers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_WALKER_S") {
      json& jj = getDef("Walkers", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_COMM") {
      json& jj = getDef("Comms", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["docs"] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_INITIALIZE_C" || nae == "INJECTION_INITIALIZE_RAW_C") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Introduction");
      jj[getPackageName(Args, 0)] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_FINALIZE") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Conclusion");
      jj[getPackageName(Args, 0)] = getDocs(Range).toJson();
    } else if (nae == "INJECTION_EXECUTABLE") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Executables");
      std::string pname = getPackageName(Args, 0);
      jj[pname] = getDocs(Range).toJson();
      jj[pname]["lib"] = "executables";
      try {
        std::string def_file = getPackageName(Args,1,true);
        jj[pname]["default"] = json::parse(def_file);
      } catch(...) {
        std::cout << "Could not parse default input file for executable " << pname;
      }
    } else if (nae == "INJECTION_LIBRARY") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Executables");
      std::string pname = getPackageName(Args, 0);
      jj[pname] = getDocs(Range).toJson();
      jj[pname]["lib"] = "libraries";
    } else if (nae == "INJECTION_PLUGIN") {
      json& jj = VnV::JsonUtilities::getOrCreate(thisJson, "Executables");
      std::string pname = getPackageName(Args, 0);
      jj[pname] = getDocs(Range).toJson();
      jj[pname]["lib"] = "plugins";
    } else if (nae == "INJECTION_LOOP_BEGIN") {
      json& jj = getDef("InjectionPoints", getPackageName(Args, 0), getPackageName(Args, 2));
      jj["docs"] = getDocs(Range).toJson();
      if (nae == "INJECTION_LOOP_BEGIN") {
        jj["loop"] = true;
      }

      //Can prob delete these -- dont think we use them anymore. Maybe in Printer.cpp
      json& stages = VnV::JsonUtilities::getOrCreate(jj, "stages");
      json& thisStage = VnV::JsonUtilities::getOrCreate(stages, "Begin");
      thisStage["docs"] = ProcessedComment().toJson();
    
  
    } else if (nae == "INJECTION_CODEBLOCK_START") {
      SourceManager& SM = pp.getSourceManager();
      SourceLocation loc = Range.getEnd();
      SourceLocation macroSrcLoc = SM.getExpansionLoc(loc);
      std::pair<FileID, unsigned> macroLoc = SM.getDecomposedLoc(macroSrcLoc);

      json& jj = getDef("CodeBlocks", getPackageName(Args, 0), getPackageName(Args, 1));
      jj["start"] = macroLoc.second + 1;

    } else if (nae == "INJECTION_CODEBLOCK_END") {
      SourceManager& SM = pp.getSourceManager();
      SourceLocation loc = Range.getBegin();
      SourceLocation macroSrcLoc = SM.getExpansionLoc(loc);
      std::pair<FileID, unsigned> macroLoc = SM.getDecomposedLoc(macroSrcLoc);

      bool Invalid = false;
      const char* Buffer = SM.getBufferData(macroLoc.first, &Invalid).data();

      if (!Invalid) {
        json& jj = getDef("CodeBlocks", getPackageName(Args, 0), getPackageName(Args, 1));
        auto start = jj["start"].get<unsigned>();
        std::string code(Buffer + start, macroLoc.second - start);
        jj["end"] = macroLoc.second;
        jj["code"] = code;
      }
    }
  }

  void MacroDefined(const Token& MacroNameTok, const MacroDirective* MD) override {
    if (active) return;
    std::string nae = pp.getSpelling(MacroNameTok);
    // if #define VNV_INCLUDED is in the file, then turn on the preprocessor
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

  bool BeginInvocation(CompilerInstance& CI) override {
    // Weird bug in Clang -- When we change directories, the file manager seems to end up in
    // the wrong directory. Resetting the file manager here means a new one will be created. This
    // new one is created in the correct directory, so we can continue.
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

  void finalize() { }

 private:
  std::unique_ptr<PreProcessVnV> ptr;
  json& mainJson;
};

json runPreprocessor(CompilationDatabase& comps, std::set<std::string>& files) {
  // Generate the main VnV Declares object.
  std::vector<std::string> files_vec(files.begin(), files.end());

  ClangTool VnVTool(comps, files_vec);
  json j = json::object();
  VnVPackageFinderFrontendActionFactory factory(j);
  VnVTool.setRestoreWorkingDir(true);
  VnVTool.run(&factory);
  factory.finalize();
  return j;
}

std::istream& vnv_getline(std::ifstream& ifs, std::string& line, int& lineno) {
  lineno++;
  return std::getline(ifs, line);
}

json runFortranPreprocessor(clang::tooling::CompilationDatabase& comps, std::set<std::string>& files) {
  std::regex initialize("vnv_initialize\\(\\\"(.*?)\\\"");
  std::regex finalize(R"(vnv_finalize\(\"(.*?)\")");
  std::regex loop_init(R"(vnv_(point|plug|loop|iterator)_init\(\s*[0,1]\s*,\s*"(.*?)\"\s*,\s*"(.*?)\")");
  std::regex parameter(
      R"(vnv_declare_(float|integer|string|double)_(parameter|array|matrix)\(\s*(.*?)\s*,\s*"(.*?)\"\s*)");
  std::regex run(R"(vnv_(point|plug|iterator|loop)_run\()");
  std::regex filesearch(R"(vnv_(input|output)_file\(\s*[0,1]\s*,\s*"(.*?)\"\s*,\s*"(.*?)\")");
  std::regex options(R"(!\s+@vnv_options_schema\((.*?)\))");
  std::regex subpackage(R"(!\s+@vnv_subpackage\((.*?),(.*?)\))");

  // Fortran processor is pretty junky at the moment -- We dont support the preprocessor -- so includes are not
  // included. For now we just assume all files compiled are the files we need to look at.

  json info = json::object();

  for (auto it : files) {
    json fileInfo = json::object();

    std::ifstream ifs(it);
    if (ifs.good()) {
      int lineNo = -1;
      std::string line;
      std::string comments;
      std::smatch match;

      json fileInfo = json::object();

      while (vnv_getline(ifs, line, lineNo)) {
        VnV::StringUtils::trim(line);

        if (std::regex_search(line, match, options)) {
          std::string opt_schema = "";
          auto package = match[1].str();
          while (vnv_getline(ifs, line, lineNo)) {
            auto lt = VnV::StringUtils::trim_copy(line);
            if (line.substr(0, 1).compare("!") == 0) {
              opt_schema += line.substr(1);
            } else {
              break;
            }
          }
          try {
            auto scheme = json::parse(opt_schema);
            json& options = VnV::JsonUtilities::getOrCreate(fileInfo, "Options");
            options[package] = json::object();
            options[package]["docs"] = processComment(comments).toJson();
            options[package]["schema"] = scheme;

            // Clear the comments since we used them.
            comments.clear();

          } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            std::cout << "Invalid Options schema -- not real json: \n\n " << opt_schema << std::endl;
          }

          // We only checked that the last line was not a comment -- so, we still need
          // to process it.
        }

        if (std::regex_search(line, match, subpackage)) {
          std::string package = match[1].str();
          std::string subs = match[2].str();
          json& options = VnV::JsonUtilities::getOrCreate(fileInfo, "SubPackages");

          json j = json::object();
          j["packageName"] = package;
          j["name"] = subs;
          j["docs"] = processComment(comments).toJson();
          comments.clear();
          continue;
        }

        if (line.size() > 1 && line.substr(0, 1).compare("!") == 0) {
          // Line is a comment so store it.
          comments += line.substr(1) + "\n";
          continue;

        } else {
          if (std::regex_search(line, match, initialize)) {
            std::string package = match[1].str();

            json& intro = VnV::JsonUtilities::getOrCreate(fileInfo, "Introduction");
            intro[package] = processComment(comments).toJson();

          } else if (std::regex_search(line, match, loop_init)) {
            std::string type = match[1].str();
            std::string package = match[2].str();
            std::string name = match[3].str();

            json j = json::object();
            j["name"] = name;
            j["packageName"] = package;

            // Comment
            j["docs"] = processComment(comments).toJson();

            // Type
            if (type.compare("plug") == 0) {
              j["plug"] = true;
            } else if (type.compare("iterator") == 0) {
              j["iterator"] = true;
            }

            // Parameters
            json parameters = json::object();
            while (vnv_getline(ifs, line, lineNo) && !std::regex_search(line, match, run)) {
              if (std::regex_search(line, match, parameter)) {
                parameters[match[4].str()] = match[1].str();
              }
            }
            j["parameters"] = json::object();
            j["parameters"][""] = parameters;

            // Stages (just fake em as we cant extract them yet):
            j["stages"] =
                R"({"Begin":{"docs":{"description":"","instructions":"","params":{},"template":"","title":""}}})"_json;
            if (type.compare("loop") == 0) {
              j["stages"]["End"] =
                  R"({"docs":{"description":"","instructions":"","params":{},"template":"","title":""}})"_json;
            }

            json in = json::object();
            in["Calling Function"] = "Unknown";
            in["Calling Function Column"] = 0;
            in["filename"] = it;
            in["lineNumber"] = lineNo;
            in["lineColumn"] = 0;

            j["info"] = in;

            json& typeJson = VnV::JsonUtilities::getOrCreate(fileInfo, "InjectionPoints");
            typeJson[package + ":" + name] = j;

          } else if (std::regex_search(line, match, filesearch)) {
            std::string type = match[1].str();
            std::string package = match[2].str();
            std::string name = match[3].str();

            json j = json::object();
            j["name"] = name;
            j["packageName"] = package;
            j["docs"] = processComment(comments).toJson();
            json& typeJson = VnV::JsonUtilities::getOrCreate(fileInfo, "Files");
            typeJson[package + ":" + name] = j;
          } else {
            // Normal line.
          }
          comments.clear();
        }
      }
      info[it] = fileInfo;
    }
  }
  return info;
  ;
}
