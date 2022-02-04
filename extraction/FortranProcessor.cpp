
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

nlohmann::json runFortranPreprocessor(clang::tooling::CompilationDatabase& comps, std::set<std::string>& files) {
  return nlohmann::json::object();
}
