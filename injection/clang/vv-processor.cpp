
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
    
   virtual void run(const MatchFinder::MatchResult &Result) {
    ASTContext *Context = Result.Context;
    if (const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite")) { 
	 if( const FunctionDecl *F = Result.Nodes.getNodeAs<clang::FunctionDecl>("function")) {
        
       
      FullSourceLoc FullLocation = Context->getFullLoc(E->getBeginLoc());
      FullSourceLoc flocation = Context->getFullLoc(F->getBeginLoc());
      if (FullLocation.isValid()) {
        
	 json info;
	 info["lineNumber"] = FullLocation.getSpellingLineNumber();
	 info["lineColumn"] = FullLocation.getSpellingColumnNumber();
         info["filename"] = "not-implemented-yet"; 
	 info["Calling Function"] = F->getNameInfo().getAsString();

	 info["InjectionPointName"] = "not-implemented-yet";
	 info["InjectionPointStage"] = "not-implemented-yet";
	 info["Number Parameters"] = "not-implemented-yet";         

	 llvm::outs() << "Found call at " << FullLocation.getSpellingLineNumber()
                     << ":" << FullLocation.getSpellingColumnNumber() << "\n";
      }
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
                 OptionsParser.getSourcePathList());

  VnVPrinter Printer;
  MatchFinder Finder;

  StatementMatcher functionMatcher =
      callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("VnV_injectionPoint")))).bind("callsite");
  Finder.addMatcher(functionMatcher, &Printer);
  return Tool.run(newFrontendActionFactory(&Finder).get());
}
