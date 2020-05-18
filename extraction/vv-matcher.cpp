
/**
 *  @file vv-matcher.cpp
 *  Clang tool to detect VnV Injection points in the Clang AST. This
 *  Tool searches the AST for calls to the VnV Injection point library and
 *  prints out a json specifying all the injection points in the class.
 *
 *  https://jonasdevlieghere.com/understanding-the-clang-ast/
 */
#include <fstream>
#include <iostream>
#include <regex>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "json-schema.hpp"
#include <algorithm>
#include <cctype>
#include <locale>
#include "base/Utilities.h"

using nlohmann::json;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;



class VnVPrinter : public MatchFinder::MatchCallback {
private:
   json& docInfo;
   json ip_json;
public:

   VnVPrinter(json &docs_) : docInfo(docs_) {
     ip_json = json::object();
   }

   json& get() {
       return ip_json;
   }

   std::string getValueFromStringLiteral(const Expr *a) {
        std::string xx = a->getStmtClassName();
        if (xx == "StringLiteral") {
            return ((const clang::StringLiteral*)a)->getString();
        } else {
                llvm::errs() << "Error Not a String Literal ";
                a->dump();
                return "";
        }
   }


   unsigned int  getInfo(const CallExpr *call, const FunctionDecl *func, const MatchFinder::MatchResult &Result , json &info, std::string &id, bool begin)
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
       unsigned int count = (begin) ? 1 : 0 ;
       std::string package =  VnV::StringUtils::trim_copy(getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts()));

       // Get the id, and create a new object in the main json for it, if needed.
       // Note: it might already be there (from Docs, or another iterations, stage, etc)
       id = VnV::StringUtils::trim_copy(getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts()));
       std::string key = package + ":" + id ;
       if (!ip_json.contains(key)) {
           if (docInfo.contains(file)) {
               if (docInfo[file]["InjectionPoints"].contains(id)) {
                  ip_json[key] = docInfo[file]["InjectionPoints"][id];
               }
           } else {
               ip_json[key] = json::object();
           }
           ip_json[key]["name"] = id;
           ip_json[key]["package"] = package;

       }
       id = key;
       if (begin) count++;
       return count;
   }


   bool extractParameters(const CallExpr *E, unsigned int count, json &parameters)
   {
       bool ret = false;
       while (count < E->getNumArgs()-1) {
           const clang::Expr *a = E->getArg(count++)->IgnoreParenCasts();
           std::string xs=getValueFromStringLiteral(a);
           const clang::Expr *aa = E->getArg(count++)->IgnoreParenCasts();

           if (aa->getType()->isDependentType())
               ret = true;

           std::string ax = aa->getType().getAsString();
           parameters[xs] = ax ;
       }
       return ret;
   }

   void addParameters(const CallExpr *E, json &idJson, unsigned int count) {
      json params;
      if ( extractParameters(E,count, params) ) {
        idJson["templateParameters"] = params;
      } else {
          json &possibleParams = VnV::JsonUtilities::getOrCreate(idJson, "parameters", VnV::JsonUtilities::CreateType::Array);
          possibleParams.push_back(params);
      }

   }

   // ** id -> {"linenumber, filename, columnnumber caller, callline, callfunc, parameters[...] , stages[iterid][info]

   virtual void run(const MatchFinder::MatchResult &Result) {
       std::cout << "HEREEEEEEEEEEEEEEEEEEEEEEEEE" << std::endl;
       std::string id;
       json info;
       if (const CXXRecordDecl *GG = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("test")) {
           std::cout  << "Found a Test" << std::endl;
           GG->dump();
       } else if (const CXXRecordDecl *GG = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("unitTest")) {
           std::cout  << "Found a Unit Test" << std::endl;
           GG->dump();
       } else if (const CXXRecordDecl *GG = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("serializer")) {
           std::cout  << "Found a Serializer" << std::endl;
           GG->dump();
       }else if (const CXXRecordDecl *GG = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("engine")) {
           std::cout  << "Found a Engine" << std::endl;
           GG->dump();
       }  else if (const CXXRecordDecl *GG = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("transform")) {
           std::cout  << "Found a Transform" << std::endl;
           GG->dump();
       } else {

       const FunctionDecl *FF = Result.Nodes.getNodeAs<clang::FunctionDecl>("function");

       if (const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite")) {
            unsigned int count = getInfo(E,FF,Result,info, id,true);
            json &idJson = VnV::JsonUtilities::getOrCreate(ip_json, id, VnV::JsonUtilities::CreateType::Object);
            json &singleJson = VnV::JsonUtilities::getOrCreate(idJson, "Begin", VnV::JsonUtilities::CreateType::Object);
            singleJson["Info"] = info;
            addParameters(E,idJson,count);
       }
       else if (const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_begin")) {
            unsigned int count = getInfo(E,FF,Result,info, id,true);
            json &idJson = VnV::JsonUtilities::getOrCreate(ip_json, id, VnV::JsonUtilities::CreateType::Object);
            json &singleJson = VnV::JsonUtilities::getOrCreate(idJson, "Begin", VnV::JsonUtilities::CreateType::Object);
            singleJson["Info"] = info;
            addParameters(E,idJson,count);
       } else if (const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_iter")) {

            unsigned int count = getInfo(E,FF,Result,info, id,false);
            std::string iterid = VnV::StringUtils::trim_copy(getValueFromStringLiteral(E->getArg(count++)->IgnoreParenCasts()));
            json &idJson = VnV::JsonUtilities::getOrCreate(ip_json, id, VnV::JsonUtilities::CreateType::Object);
            json &stagesJson = VnV::JsonUtilities::getOrCreate(idJson, "stages", VnV::JsonUtilities::CreateType::Object);
            json &thisStageJson = VnV::JsonUtilities::getOrCreate(stagesJson, iterid, VnV::JsonUtilities::CreateType::Object);
            thisStageJson["Info"] = info;
       } else if ( const CallExpr *E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_end")) {
           getInfo(E,FF,Result,info, id,false);
           json &idJson = VnV::JsonUtilities::getOrCreate(ip_json, id, VnV::JsonUtilities::CreateType::Object);
           json &singleJson = VnV::JsonUtilities::getOrCreate(idJson, "End", VnV::JsonUtilities::CreateType::Object);
           singleJson["Info"] = info;

       }else {
           std::cout<< "Failded to match the match" << std::endl;
       }
}
   }

};

class VnVCommentParser {
public :

    std::set<std::string> matchedFiles;
    std::set<std::string> getAllFiles() {
        return matchedFiles;
    }


    json& getOrCreate(json &j, std::string name) {
        if (!j.contains(name)) {
            json jj;
            j[name] = json::object();
        }
        return j[name];
    }


    void parse(json &coreJson, std::string filename) {

        std::ifstream stream;
        stream.open(filename);
        std::stringstream oss;
        oss << stream.rdbuf();
        std::string searchstring = oss.str();


        std::regex r("(?:\\/\\*\\*((?:[^*]|[\r\n]|(?:\\*+(?:[^*/]|[\r\n])))*)\\*+\\/(?:[ \t\r\n]+))?(?:INJECTION_(TEST_R|TEST|SERIALIZER_R|SERIALIZER|UNITTEST_R|UNITTEST|TRANSFORM_R|TRANSFORM|POINT_C|POINT|LOOP_BEGIN_C|LOOP_BEGIN|LOOP_END|LOOP_ITER|FUNCTION_WRAPPER_C|FUNCTION_WRAPPER)*)\\(((.*?)(?:,(.*?))?(?:,(?:.*?))*)\\)");
        std::smatch match;
        std::string::const_iterator searchStart(searchstring.cbegin());

	json& tests = getOrCreate(coreJson,"Tests");
	json& inject = getOrCreate(coreJson,"InjectionPoints");
	json& unitTests = getOrCreate(coreJson,"UnitTests");
	json& transforms = getOrCreate(coreJson,"Transforms");
	json& serializers = getOrCreate(coreJson,"Serializers");


        while (std::regex_search(searchStart, searchstring.cend(), match, r)) {

            // Should match both documented and undocuemented INJECTION points.
            //Match 0 --> full match
            //Match 1 --> Comment contents
            //Match 2 --> "" | "_ITER" | "_BEGIN" | "_END" ...
            //Match 3 --> comma seperated list of all the parameters (as a string, no parens. (i.e., par1,par2,par3,... )
            //Match 4 --> parameter1
            //Match 5 --> parameter2
            //Match 6 --> parameter3
            // ...

            std::string docs = ((match[1]).length()==0 ) ? "<None>" : match[1].str();
            if (match[2] == "LOOP_ITER") {
                std::string m = VnV::StringUtils::trim_copy(match[4]);
                json& thisJson = getOrCreate(inject,m);
                json& iters = getOrCreate(thisJson,"stages");
                std::string itername = VnV::StringUtils::trim_copy(match[5]);
                json& stage = getOrCreate(iters,itername);
                stage["Docs"] = docs;
            } else if (match[2] == "LOOP_END") {
                std::string m = VnV::StringUtils::trim_copy(match[4]);
                json& thisJson = getOrCreate(inject,m);
                json &end = getOrCreate(thisJson,"End");
                end["Docs"] = docs;
            } else if (match[2].str().substr(0,4) == "TEST" ) {
                std::string testName = VnV::StringUtils::trim_copy(match[4]);
                json& thisJson = getOrCreate(tests, testName);
                thisJson["name"] = testName;
                thisJson["docs"] = docs;
                json params = getOrCreate(thisJson,"params");

		// If match[2] == TEST_R then skip over the runner.
		// TODO regex DOES not match things like std::map<sdfs,sdfsdf>
		// REQUIRE TYPES WITH COMMAS TO BE WRAPPED IN PARENTHESIS
		std::size_t count = ( match[2].str().size() == 4) ? 5 : 6 ;
		std::string curr = "";
		while (count < match.size() ) {
		   curr = curr + match[count++].str();
		   if (VnV::StringUtils::balancedParenthesis(curr)) {
			auto pa = VnV::StringUtils::splitCppArgString(curr);
			params[pa.first] = pa.second;
			curr = "";
		   } else if (count == match.size()) {
			//Error unbalenced parens at end of string. (INVALID)
			std::abort();
		   }
		}
	    } else if (match[2].str().substr(0,8) == "UNITTEST") {
	       std::string testName = VnV::StringUtils::trim_copy(match[4].str());
	       json& thisJson = getOrCreate(unitTests, testName);
	       thisJson["docs"] = docs;
	    } else if (match[2].str().substr(0,9) == "TRANSFORM") {
	       std::string testName = VnV::StringUtils::trim_copy(match[4].str());
	       json& thisJson = getOrCreate(transforms, testName);
	       thisJson["docs"] = docs;

	       // Extract the from and to. If the type includes commas,
	       // the match wont be complete. The work around for this is
	       // to iterate over the match until the parenthesis are balencd.
	       // ALL TYPES with COMMAS need to be wrapped in parenthesis.
	       std::size_t count = (match[2].str().size() == 9) ? 5 : 6;
	       std::string find = "from";
	       std::string curr = "";
	       while (count < match.size() ) {
		   curr = curr + match[count++].str();
		   if (VnV::StringUtils::balancedParenthesis(curr)) {
			thisJson[find] = curr;
			curr = "";
			find = "to";
		   } else if (count == match.size()) {
			//Error unbalenced parens at end of string. (INVALID)
			std::abort();
		   }
		}
	    } else if (match[2].str().substr(0,9) == "SERIALIZER") {
	       std::string testName = VnV::StringUtils::trim_copy(match[4].str());
	       json& thisJson = getOrCreate(serializers, testName);
	       thisJson["docs"] = docs;

	       // Extract the type. If the type includes commas,
	       // the match wont be complete. The work around for this is
	       // to iterate over the match until the parenthesis are balencd.
	       // ALL TYPES with COMMAS need to be wrapped in parenthesis.
	       std::size_t count = (match[2].str().size() == 10 ) ? 5 : 6;
	       std::string find = "from";
	       std::string curr = "";
	       while (count < match.size() ) {
		   curr = curr + match[count++].str();
	       }
	       thisJson["type"] = curr;

            }  else {
                std::string m = VnV::StringUtils::trim_copy(match[5].str());
                json& thisJson = getOrCreate(inject,m);
                json &begin = getOrCreate(thisJson,"Begin");
                begin["Docs"] = docs;
            }

            matchedFiles.insert(filename);
            searchStart = match.suffix().first;
       }
       matchedFiles.insert(filename);
       // Need to match any file that also contains the ITest, IUnitTest,
    }

    json parseFiles( std::vector<std::string> filename) {
        json j;

        std::string x = __FILE__;
        for (auto it : filename) {
            if (it != x ) {
                json js = json::object();
                parse(js,it);
                j[it] = js;
            }
        }
        return j;
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


/** CommonOptionsParser declares HelpMessage with a description of the common
 command-line options related to the compilation database and input files.
 It's nice to have this help message in all tools.
**/
//static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);


/**
 * A help message for this specific tool can be added afterwards.
**/
static cl::extrahelp MoreHelp("\nMore help text...");

/**
 * Main Executable for VnV Processor.
 */
int main(int argc, const char **argv) {


  CommonOptionsParser OptionsParser(argc, argv, VnVParserCatagory);

  VnVCommentParser parser;
  json info = parser.parseFiles(OptionsParser.getCompilations().getAllFiles());

  std::string outputFileName = outputFile.getValue();

  std::set<std::string> sfiles = parser.getAllFiles();
  std::vector<std::string> s(sfiles.begin(),sfiles.end());

  ClangTool Tool(OptionsParser.getCompilations(), s);

  VnVPrinter Printer(info);
  MatchFinder Finder;
  /** C Matchers **/
  StatementMatcher functionMatcher = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint")))).bind("callsite");
  StatementMatcher functionMatcher1 = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint_begin")))).bind("callsite_begin");
  StatementMatcher functionMatcher2 = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint_loop")))).bind("callsite_iter");
  StatementMatcher functionMatcher3 = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("_VnV_injectionPoint_end")))).bind("callsite_end");

  Finder.addMatcher(functionMatcher, &Printer);
  Finder.addMatcher(functionMatcher1, &Printer);
  Finder.addMatcher(functionMatcher2, &Printer);
  Finder.addMatcher(functionMatcher3, &Printer);

  /** Cpp Matchers **/
  StatementMatcher functionMatcherC = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("VnV::CppInjection::BeginPack")))).bind("callsite");
  StatementMatcher functionMatcher1C = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("VnV::CppInjection::BeginLoopPack")))).bind("callsite_begin");
  StatementMatcher functionMatcher2C = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("VnV::CppInjection::IterLoop")))).bind("callsite_iter");
  StatementMatcher functionMatcher3C = callExpr(hasAncestor(functionDecl().bind("function")),callee(functionDecl(hasName("VnV::CppInjection::EndLoop")))).bind("callsite_end");
  Finder.addMatcher(functionMatcherC, &Printer);
  Finder.addMatcher(functionMatcher1C, &Printer);
  Finder.addMatcher(functionMatcher2C, &Printer);
  Finder.addMatcher(functionMatcher3C, &Printer);


  int x = Tool.run(newFrontendActionFactory(&Finder).get());


  std::cout << Printer.get().dump(3) << std::endl;

  std::ofstream oss(outputFileName);
  oss << Printer.get().dump(3);
  oss.close();

  return 1;//x;

}
