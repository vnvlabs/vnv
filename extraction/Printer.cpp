#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <iostream>
#include <string>

#include "base/Utilities.h"
#include "base/exceptions.h"
using namespace clang;
using namespace llvm;
using namespace clang::ast_matchers;
using nlohmann::json;

namespace {

std::string getValueFromStringLiteral(const Expr* a) {
  std::string xx = a->getStmtClassName();
  if (xx == "StringLiteral") {
    return ((const clang::StringLiteral*)a)->getString().str();
  } else {
    llvm::errs() << "Error Not a String Literal ";
    a->dump();
    throw VnV::VnVExceptionBase("SDFSDF");
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
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);

    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("callsite_begin")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);

    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("cpp_callsite")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);

    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>(
                   "cpp_callsite_iteration")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // skip the filename
      count++;  // skip the line number
      count++;  // skip the callback
      count++;  // skip the once parameter
      count++;  // skip the inputs parameter;

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);

    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>(
                   "cpp_callsite_plug")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback
      count++;  // skip the input parameter count

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);

    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>(
                   "callsite_iteration")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // skip the filename
      count++;  // skip the line number
      count++;  // skip the callback
      count++;  // skip the once parameter
      count++;  // skip the inputs parameter;

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(E, idJson, count);

    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>(
                   "cpp_callsite_begin")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;

      addParameters(E, idJson, count);
    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("callsite_iter")) {
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 0);

      std::string iterid = VnV::StringUtils::trim_copy(
          getValueFromStringLiteral(E->getArg(count++)->IgnoreParenCasts()));
      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& stagesJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      stagesJson[iterid] = info;

    } else if (const CallExpr* E =
                   Result.Nodes.getNodeAs<clang::CallExpr>("callsite_end")) {
      getInfo(E, FF, Result, info, id, filename, 0);
      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["End"] = info;
    }
  }
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
    StatementMatcher functionMatcher4 =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("_VnV_injectionIteration"))))
            .bind("callsite_iteration");

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
    StatementMatcher functionMatcher4C =
        callExpr(
            hasAncestor(functionDecl().bind("function")),
            callee(functionDecl(hasName("VnV::CppIteration::IterationPack"))))
            .bind("cpp_callsite_iteration");
    StatementMatcher functionMatcher3C =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("VnV::CppInjection::EndLoop"))))
            .bind("callsite_end");

    StatementMatcher functionMatcher5C =
        callExpr(hasAncestor(functionDecl().bind("function")),
                 callee(functionDecl(hasName("VnV::CppPlug::PlugPack"))))
            .bind("cpp_callsite_plug");

    addMatcher(functionMatcherC, &Printer);
    addMatcher(functionMatcher1C, &Printer);
    addMatcher(functionMatcher2C, &Printer);
    addMatcher(functionMatcher3C, &Printer);
    addMatcher(functionMatcher4C, &Printer);
    addMatcher(functionMatcher5C, &Printer);

    addMatcher(functionMatcher, &Printer);
    addMatcher(functionMatcher1, &Printer);
    addMatcher(functionMatcher2, &Printer);
    addMatcher(functionMatcher3, &Printer);
    addMatcher(functionMatcher4, &Printer);
  }
  json& get() { return Printer.get(); }
};

json runFinder(clang::tooling::CompilationDatabase& db,
               std::vector<std::string>& files) {
  // Search the AST to extract information about injection points. In
  // particular,
  // we search the AST to extract the parameter types.
  VnVFinder Finder;
  clang::tooling::ClangTool Tool(db, files);
  Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());
  return Finder.get();
}
