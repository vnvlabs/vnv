#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/Version.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

#include <iostream>
#include <string>

#include "shared/Utilities.h"
#include "shared/exceptions.h"
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
    throw INJECTION_EXCEPTION("Could not extract value for expression as it is a %s and not a StringLiteral",
                               xx.c_str());
  }
}

enum class Compiler { CLANG11, CLANG12, GNU };

// Rewrite this function so clang prints in GCC Format.
std::string ComputeName(const clang::Decl* CurrentDecl, Compiler compiler) {
  clang::ASTContext& Context = CurrentDecl->getASTContext();

  if (const FunctionDecl* FD = dyn_cast<FunctionDecl>(CurrentDecl)) {
    SmallString<256> Name;
    llvm::raw_svector_ostream Out(Name);

    if (const CXXMethodDecl* MD = dyn_cast<CXXMethodDecl>(FD)) {
      if (MD->isStatic()) Out << "static ";
    }

    PrintingPolicy Policy(Context.getLangOpts());
    std::string Proto;
    llvm::raw_string_ostream POut(Proto);

    const FunctionDecl* Decl = FD;

    if (const FunctionDecl* Pattern = FD->getTemplateInstantiationPattern()) {
      Decl = Pattern;
    }

    const clang::FunctionType* AFT = Decl->getType()->getAs<clang::FunctionType>();
    const FunctionProtoType* FT = nullptr;

    if (FD->hasWrittenPrototype()) FT = dyn_cast<FunctionProtoType>(AFT);

    FD->printQualifiedName(POut, Policy);

    POut << "(";
    if (FT) {
      for (unsigned i = 0, e = Decl->getNumParams(); i != e; ++i) {
        if (i) POut << ", ";
        POut << Decl->getParamDecl(i)->getType().stream(Policy);
      }

      if (FT->isVariadic()) {
        if (FD->getNumParams()) POut << ", ";
        POut << "...";
      } else if ((!Context.getLangOpts().CPlusPlus) && !Decl->getNumParams()) {
        POut << "void";
      }
    }
    POut << ")";

    if (const CXXMethodDecl* MD = dyn_cast<CXXMethodDecl>(FD)) {
      assert(FT && "We must have a written prototype in this case.");
      if (FT->isConst()) POut << " const";
      if (FT->isVolatile()) POut << " volatile";
      RefQualifierKind Ref = MD->getRefQualifier();
      if (Ref == RQ_LValue)
        POut << " &";
      else if (Ref == RQ_RValue)
        POut << " &&";
    }

    typedef SmallVector<const ClassTemplateSpecializationDecl*, 8> SpecsTy;
    SpecsTy Specs;
    const DeclContext* Ctx = FD->getDeclContext();
    while (Ctx && isa<NamedDecl>(Ctx)) {
      const ClassTemplateSpecializationDecl* Spec = dyn_cast<ClassTemplateSpecializationDecl>(Ctx);
      if (Spec && !Spec->isExplicitSpecialization()) Specs.push_back(Spec);
      Ctx = Ctx->getParent();
    }

    std::string TemplateParams;
    llvm::raw_string_ostream TOut(TemplateParams);
    for (const ClassTemplateSpecializationDecl* D : llvm::reverse(Specs)) {
      const TemplateParameterList* Params = D->getSpecializedTemplate()->getTemplateParameters();
      const TemplateArgumentList& Args = D->getTemplateArgs();
      assert(Params->size() == Args.size());
      for (unsigned i = 0, numParams = Params->size(); i != numParams; ++i) {
        StringRef Param = Params->getParam(i)->getName();
        if (Param.empty()) continue;
        TOut << Param << " = ";
        Args.get(i).print(Policy, TOut);
        TOut << ", ";
      }
    }

    std::string sep = (compiler == Compiler::GNU) ? "; " : ", ";

#if CLANG_VERSION_MAJOR > 11
    Policy.SuppressDefaultTemplateArgs = !(compiler == Compiler::CLANG11);
#endif

    std::string with = (compiler == Compiler::GNU) ? "with" : "";

    FunctionTemplateSpecializationInfo* FSI = FD->getTemplateSpecializationInfo();
    if (FSI && !FSI->isExplicitSpecialization()) {
      const TemplateParameterList* Params = FSI->getTemplate()->getTemplateParameters();
      const TemplateArgumentList* Args = FSI->TemplateArguments;
      assert(Params->size() == Args->size());
      for (unsigned i = 0, e = Params->size(); i != e; ++i) {
        StringRef Param = Params->getParam(i)->getName();
        if (Param.empty()) continue;
        TOut << Param << " = ";

        Args->get(i).print(Policy, TOut);
        TOut << sep;
      }
    }

    TOut.flush();
    if (!TemplateParams.empty()) {
      // remove the trailing comma and space
      TemplateParams.resize(TemplateParams.size() - 2);
      POut << "[" << with << TemplateParams << "]";
    }

    POut.flush();

    // Print "auto" for all deduced return types. This includes C++1y return
    // type deduction and lambdas. For trailing return types resolve the
    // decltype expression. Otherwise print the real type when this is
    // not a constructor or destructor.
    if (isa<CXXMethodDecl>(FD) && cast<CXXMethodDecl>(FD)->getParent()->isLambda())
      Proto = "auto " + Proto;
    else if (FT && FT->getReturnType()->getAs<DecltypeType>())
      FT->getReturnType()->getAs<DecltypeType>()->getUnderlyingType().getAsStringInternal(Proto, Policy);
    else if (!isa<CXXConstructorDecl>(FD) && !isa<CXXDestructorDecl>(FD))
      AFT->getReturnType().getAsStringInternal(Proto, Policy);

    Out << Proto;

    return std::string(Name.c_str());
  }
  return "unknown";
}

std::string ComputeName(std::string compiler, const Decl* func) {
  if (compiler.compare("GNU") == 0) {
    return ComputeName(func, Compiler::GNU);
  } else if (compiler.compare("CLANG11") == 0) {
    return ComputeName(func, Compiler::CLANG11);
  } else {
    return ComputeName(func, Compiler::CLANG12);
  }
}

std::string getSig(const CallExpr* call, int count, const FunctionDecl* caller) {
  auto a = call->getArg(count);
  const clang::Expr* bb = nullptr;

  if (a->getStmtClass() == clang::Stmt::StmtClass::CallExprClass) {
    bb = ((const clang::CallExpr*)a)->getArg(0);
  } else if (a->getStmtClass() == clang::Stmt::StmtClass::CXXConstructExprClass) {
    auto c = ((const clang::CXXConstructExpr*)a)->getArg(0);
    if (c->getStmtClass() == clang::Stmt::StmtClass::MaterializeTemporaryExprClass) {
      auto d = ((const clang::MaterializeTemporaryExpr*)c)->getSubExpr();

      if (d->getStmtClass() == clang::Stmt::StmtClass::CallExprClass) {
        bb = ((const clang::CallExpr*)d)->getArg(0);
      }
    }
  }

  while (bb != nullptr && bb->getStmtClass() == clang::Stmt::StmtClass::ImplicitCastExprClass) {
    bb = ((const clang::ImplicitCastExpr*)bb)->getSubExpr();
  }
  if (bb->getStmtClass() == clang::Stmt::StmtClass::StringLiteralClass) {
    std::string compiler = ((const clang::StringLiteral*)bb)->getString().str();
    return ComputeName(compiler, caller);
  }
  throw INJECTION_EXCEPTION(
      "Error extracting Function Signiture. The parameter is not a string literal class (it is %s)",
      bb->getStmtClassName());
}

std::string stripFilename(std::string filename, std::string strip) {
  if (filename.find(strip) == 0) {
    return filename.substr(strip.size());
  }
  return filename;
}

unsigned int getInfo(const CallExpr* call, const FunctionDecl* func, const MatchFinder::MatchResult& Result, json& info,
                     std::string& id, std::string& filename, int begin, std::string strip) {
  FullSourceLoc callLocation = Result.Context->getFullLoc(call->getBeginLoc());
  FullSourceLoc funclocation = Result.Context->getFullLoc(func->getBeginLoc());

  info["lineNumber"] = callLocation.getSpellingLineNumber();
  info["lineColumn"] = callLocation.getSpellingColumnNumber();

  SourceManager& srcMgr = Result.Context->getSourceManager();
  filename = srcMgr.getFilename(func->getLocation()).str();

  info["filename"] = stripFilename(filename, strip);  // Result.SourceManager->getFilename(callLocation);

  info["Calling Function"] = func->getNameInfo().getAsString();
  info["Calling Function Line"] = funclocation.getSpellingLineNumber();
  info["Calling Function Column"] = funclocation.getSpellingColumnNumber();

  json parameters;
  unsigned int count = (begin);
  std::string package =
      VnV::StringUtils::trim_copy(getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts()));
  id = VnV::StringUtils::trim_copy(getValueFromStringLiteral(call->getArg(count++)->IgnoreParenCasts()));
  std::string key = package + ":" + id;
  id = key;
  return count;
}

json extractParameters(const CallExpr* E, unsigned int count) {
  json parameters = json::object();
  while (count < E->getNumArgs() - 1) {
    const clang::Expr* a = E->getArg(count++)->IgnoreParenCasts();
    std::string xs = getValueFromStringLiteral(a);
    const clang::Expr* aa = E->getArg(count++)->IgnoreParenCasts();
    std::string ax = VnV::StringUtils::squash_copy(aa->getType().getAsString());
    parameters[xs] = ax;
  }
  return parameters;
}

void addParameters(std::string sig, const CallExpr* E, json& idJson, unsigned int count) {
  json params = extractParameters(E, count);

  json& possibleParams = VnV::JsonUtilities::getOrCreate(idJson, "parameters", VnV::JsonUtilities::CreateType::Object);

  auto it = possibleParams.find(sig);
  if (it != possibleParams.end()) {
    if (it->dump().compare(params.dump()) != 0) {
      std::string s = it->dump();
      std::string p = params.dump();
      throw INJECTION_EXCEPTION(
          "Two injection points with same signiture and ID but different parameters were encounted\n"
          "Signuture: %s\n"
          "Option 1: %s \n"
          "Option 2: %s",
          sig.c_str(), s.c_str(), p.c_str());
    }

  } else {
    possibleParams[sig] = params;
  }
}
}  // namespace

class VnVPrinter : public MatchFinder::MatchCallback {
 private:
  json main_json;
  std::string strip;

 public:
  VnVPrinter(std::string strip_) : strip(strip_) { main_json = json::object(); }

  json& get() { return main_json; }

  virtual void run(const MatchFinder::MatchResult& Result) {
    std::string filename;
    std::string id;
    json info;

    const FunctionDecl* FF = Result.Nodes.getNodeAs<clang::FunctionDecl>("function");
    if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite")) {
      // Single IP using C interface
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1, strip);
      std::string sig = getSig(E, count++, FF);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(sig, E, idJson, count);

    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_begin")) {
      // Loop begin using the C interface
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1, strip);
      std::string sig = getSig(E, count++, FF);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(sig, E, idJson, count);

    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>("cpp_callsite")) {
      // Single IP using the C++ interface
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1, strip);

      std::string sig = getSig(E, count++, FF);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;
      addParameters(sig, E, idJson, count);

    
    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>("cpp_callsite_begin")) {
      // Loop begin with the C++ interface
      unsigned int count = getInfo(E, FF, Result, info, id, filename, 1, strip);
      std::string sig = getSig(E, count++, FF);
      count++;  // Skip the filename
      count++;  // Skip the line
      count++;  // Skip the callback

      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["Begin"] = info;

      addParameters(sig, E, idJson, count);
    } else if (const CallExpr* E = Result.Nodes.getNodeAs<clang::CallExpr>("callsite_end")) {
      // End of  a loop in the C AND C++ interface
      getInfo(E, FF, Result, info, id, filename, 0, strip);
      json& idJson = VnV::JsonUtilities::getOrCreate(main_json, id);
      json& singleJson = VnV::JsonUtilities::getOrCreate(idJson, "stages");
      singleJson["End"] = info;
    }
  }
};

class VnVFinder : public MatchFinder {
 public:
  VnVPrinter Printer;
  VnVFinder(std::string strip) : Printer(strip) {
    // Begin Single injection point in C
    StatementMatcher functionMatcher =
        callExpr(hasAncestor(functionDecl().bind("function")), callee(functionDecl(hasName("_VnV_injectionPoint"))))
            .bind("callsite");

    // Begin Loop in C
    StatementMatcher functionMatcher1 = callExpr(hasAncestor(functionDecl().bind("function")),
                                                 callee(functionDecl(hasName("_VnV_injectionPoint_begin"))))
                                            .bind("callsite_begin");

    // Loop End in C
    StatementMatcher functionMatcher3 =
        callExpr(hasAncestor(functionDecl().bind("function")), callee(functionDecl(hasName("_VnV_injectionPoint_end"))))
            .bind("callsite_end");

    // Begin Single Injection Point in C++
    StatementMatcher functionMatcherC = callExpr(hasAncestor(functionDecl().bind("function")),
                                                 callee(functionDecl(hasName("VnV::CppInjection::BeginPack"))))
                                            .bind("cpp_callsite");

    // Begin Loop in C+++
    StatementMatcher functionMatcher1C = callExpr(hasAncestor(functionDecl().bind("function")),
                                                  callee(functionDecl(hasName("VnV::CppInjection::BeginLoopPack"))))
                                             .bind("cpp_callsite_begin");

    // Loop end in C++
    StatementMatcher functionMatcher3C = callExpr(hasAncestor(functionDecl().bind("function")),
                                                  callee(functionDecl(hasName("VnV::CppInjection::EndLoop"))))
                                             .bind("callsite_end");




    addMatcher(functionMatcherC, &Printer);
    addMatcher(functionMatcher1C, &Printer);
    addMatcher(functionMatcher3C, &Printer);

    addMatcher(functionMatcher, &Printer);
    addMatcher(functionMatcher1, &Printer);
    addMatcher(functionMatcher3, &Printer);
  }
  json& get() { return Printer.get(); }
};

class VnVFinderAction : public ASTFrontendAction {
  VnVFinder& finder;

 public:
  VnVFinderAction(VnVFinder& f) : finder(f) {}

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance&, StringRef) override {
    return finder.newASTConsumer();
  }

  bool BeginInvocation(CompilerInstance& CI) override {
    // Weird bug in Clang -- When we change directories, the file manager seems to end up in
    // the wrong directory. Resetting the file manager here means a new one will be created. This
    // new one is created in the correct directory, so we can continue.
    CI.setFileManager(nullptr);
    CI.setSourceManager(nullptr);
    return true;
  }

  bool BeginSourceFileAction(CompilerInstance& CI) override {
    if (!ASTFrontendAction::BeginSourceFileAction(CI)) return false;
    return true;
  }

  void EndSourceFileAction() override { ASTFrontendAction::EndSourceFileAction(); }
};

class VnVFinderActionFactory : public tooling::FrontendActionFactory {
  VnVFinder& finder;

 public:
  VnVFinderActionFactory(VnVFinder& f) : finder(f) {}
  std::unique_ptr<FrontendAction> create() override { return std::make_unique<VnVFinderAction>(finder); }
};

json runFinder(clang::tooling::CompilationDatabase& db, std::vector<std::string>& files, std::string strip) {
  // Search the AST to extract information about injection points. In
  // particular,
  // we search the AST to extract the parameter types.
  VnVFinder Finder(strip);
  clang::tooling::ClangTool Tool(db, files);
  Tool.setRestoreWorkingDir(true);

  VnVFinderActionFactory factory(Finder);
  Tool.run(&factory);
  return Finder.get();
}
