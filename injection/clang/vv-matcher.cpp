
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
#include <fstream>
#include <iostream>

using nlohmann::json;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

class VnVPrinter : public MatchFinder::MatchCallback {
private: 
   std::string outputFileName;
   json ip_json;		
public:
    
   VnVPrinter(std::string _outputFileName) : outputFileName(_outputFileName) {
      std::ifstream fs("./vnv-out.json" ) ;
      if ( fs.is_open() ) {
	 if ( fs.peek() != std::ifstream::traits_type::eof()  ) { 
	    ip_json = json::parse(fs);
         }
      } else {
        
      }
   }	   

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
         
	

	 std::string name = "hello";
	 info["num_args"] = E->getNumArgs();
	 info["num_commas"] = E->getNumCommas();
	 for ( auto it = E->arg_begin(); it!= E->arg_end(); it++ ) {
             const Expr* ee = *it;
	     QualType q = ee->getType();

             std::cout << q.getAsString() << std::endl;      
	 } 



	 info["InjectionPointName"] = "hello";
	 info["InjectionPointStage"] = "not-implemented-yet";
	 info["Number Parameters"] = "not-implemented-yet";         
   	 ip_json["name"] = info;

      }

    }
   }
  }

  json getJson() {
    return ip_json;
  }

};

/** Apply a custom category to all command-line options so that they are the
 only ones displayed.
*/
static llvm::cl::OptionCategory VnVCategory("VnV PreCheck options");

/** CommonOptionsParser declares HelpMessage with a description of the common
 command-line options related to the compilation database and input files.
 It's nice to have this help message in all tools.
**/
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static llvm::cl::opt<std::string>
    ProjectName("VnV", llvm::cl::desc("VnV Precheck Parser"),
                llvm::cl::cat(VnVCategory));


static llvm::cl::opt<std::string> OutputDirectory(
    "vnv-output",
    llvm::cl::desc("VnV Output File Name"),
    llvm::cl::init("vnv.json"), llvm::cl::cat(VnVCategory));


/**
 * A help message for this specific tool can be added afterwards.
**/
static cl::extrahelp MoreHelp("\nMore help text...");

/**
 * Main Executable for VnV Processor. 
 */
int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, VnVCategory);
  
  
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  VnVPrinter Printer(OutputDirectory.getValue());
  MatchFinder Finder;

  StatementMatcher functionMatcher =
      callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("VnV_injectionPoint")))).bind("callsite");
  Finder.addMatcher(functionMatcher, &Printer);
  return Tool.run(newFrontendActionFactory(&Finder).get());
}
