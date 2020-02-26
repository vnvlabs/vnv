
/**
 *  @file vv-matcher.cpp
 *  Clang tool to detect VnV Injection points in the Clang AST. This 
 *  Tool searches the AST for calls to the VnV Injection point library and
 *  prints out a json specifying all the injection points in the class. 
 *
 *  https://jonasdevlieghere.com/understanding-the-clang-ast/
 */
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

#include "llvm/Support/CommandLine.h"

#include "json-schema.hpp"


using nlohmann::json;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

class VnVPrinter : public MatchFinder::MatchCallback {
private: 
   json ip_json;		
public:

   json get() {
       return ip_json;
   }

   std::string getValueFromStringLiteral(const Expr *a) {
        std::string xx = a->getStmtClassName();
        if (xx == "StringLiteral") {
            return ((const clang::StringLiteral*)a)->getString();
        } else {
                llvm::errs() << "Error Not a String Literal ";
                return "";
        }
   }


   unsigned int  getInfo(const CallExpr *call, const FunctionDecl *func, const MatchFinder::MatchResult &Result , json &info, std::string &package, std::string &id)
   {
       FullSourceLoc callLocation = Result.Context->getFullLoc(call->getBeginLoc());
       FullSourceLoc funclocation = Result.Context->getFullLoc(func->getBeginLoc());

       info["lineNumber"] = callLocation.getSpellingLineNumber();
       info["lineColumn"] = callLocation.getSpellingColumnNumber();

       SourceManager &srcMgr = Result.Context->getSourceManager();
       std::string file = srcMgr.getFilename(func->getLocation()).str();
       info["filename"] = file;// Result.SourceManager->getFilename(callLocation);


       SourceManager &  manager(Result.Context->getSourceManager());
       info["Calling Function"] = func->getNameInfo().getAsString();
       info["Calling Function Line"] = funclocation.getSpellingLineNumber();
       info["Calling Function Column"] = funclocation.getSpellingColumnNumber();

       json parameters;
       unsigned int count = 0;
       package = getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts());
       id = getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts());
       return count;
   }

   void addToMainJson(std::string package, std::string id, json info)
   {
       if (!ip_json.contains(package)) {
          ip_json[package] = json::object();
       }
       if (!ip_json[package].contains(id)) {
          ip_json[package][id] = json::object();
          ip_json[package][id]["Start"] = json::array();
       }
       ip_json[package][id]["Start"].push_back(info);
   }

   json extractParameters(const CallExpr *E, unsigned int count)
   {
       json parameters;
       while (count < E->getNumArgs()-1) {
           const clang::Expr *a = E->getArg(count++)->IgnoreParenCasts();
           std::string xs=getValueFromStringLiteral(a);
           const clang::Expr *aa = E->getArg(count++)->IgnoreParenCasts();
           std::string ax = aa->getType().getAsString();
           parameters[xs] = ax;
       }
       return parameters;
   }

   virtual void run(const MatchFinder::MatchResult &Result) {

       std::string package;
       std::string id;
       json info;

       const FunctionDecl *FF = Result.Nodes.getNodeAs<clang::FunctionDecl>("function");

       if (const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite")) {
            unsigned int count = getInfo(E,FF,Result,info, package,id);
            info["parameters"] = extractParameters(E,count);
            addToMainJson(package,id,info);
       } else if (const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_iter")) {
            unsigned int count = getInfo(E,FF,Result,info, package,id);
            std::string iterid = getValueFromStringLiteral(E->getArg(count++)->IgnoreParenCasts());
            if (ip_json.contains(package) && ip_json[package].contains(id)) {
                if(!ip_json[package][id].contains("stages")) {
                   ip_json[package][id]["stages"] = json::object();
                }
                ip_json[package][id]["stages"][iterid] = info;
            }
       } else if ( const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_end")) {
           getInfo(E,FF,Result,info, package,id);
           if (ip_json.contains(package) && ip_json[package].contains(id)) {
               ip_json[package][id]["End"] = info;
           }
       }
    }
  
};

/** Apply a custom category to all command-line options so that they are the
 only ones displayed.
*/
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

/** CommonOptionsParser declares HelpMessage with a description of the common
 command-line options related to the compilation database and input files.
 It's nice to have this help message in all tools.
**/
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);


/**
 * A help message for this specific tool can be added afterwards.
**/
static cl::extrahelp MoreHelp("\nMore help text...");

/**
 * Main Executable for VnV Processor. 
 */
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getCompilations().getAllFiles());

  VnVPrinter Printer;
  MatchFinder Finder;

  StatementMatcher functionMatcher = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint")))).bind("callsite");
  StatementMatcher functionMatcher1 = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint_begin")))).bind("callsite");
  StatementMatcher functionMatcher2 = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint_loop")))).bind("callsite_iter");
  StatementMatcher functionMatcher3 = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint_end")))).bind("callsite_end");
  Finder.addMatcher(functionMatcher, &Printer);
  Finder.addMatcher(functionMatcher1, &Printer);
  Finder.addMatcher(functionMatcher2, &Printer);
  Finder.addMatcher(functionMatcher3, &Printer);
  int x = Tool.run(newFrontendActionFactory(&Finder).get());

  llvm::outs() << Printer.get().dump(3,' ') << "\n";
  return x;

}
