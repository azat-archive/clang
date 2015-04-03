//===- unittests/AST/LeftoverExpressionsForODRAfterFatalError.cpp ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the regression for the next assert after fatal error:
// clang::Sema::ActOnFinishFunctionBody(clang::Decl*, clang::Stmt*, bool):
// Assertion `MaybeODRUseExprs.empty() && "Leftover expressions for odr-use checking"' failed.
//
// After DiagnosticsEngine::{Reset(),setIgnoreAllWarnings()}, using ASTConsumer
//
//===----------------------------------------------------------------------===//

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Frontend/CompilerInstance.h"
#include "gtest/gtest.h"

using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace {

using namespace clang;

class TestConsumer : public ASTConsumer {
public:
    TestConsumer(clang::CompilerInstance &ci)
        : clang::ASTConsumer()
        , ci(ci) {}
    virtual ~TestConsumer() {}

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef D) override {
        if (ci.getDiagnostics().hasFatalErrorOccurred()) {
            ci.getDiagnostics().Reset();
            ci.getDiagnostics().setIgnoreAllWarnings(true);
        }
        return true;
    }

private:
    clang::CompilerInstance &ci;
};

class TestAction : public ASTFrontendAction {
    virtual std::unique_ptr<clang::ASTConsumer>
    CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef /*InFile */) override {
        return std::unique_ptr<TestConsumer>(new TestConsumer(ci));
    }
};

}


TEST(Decl, LeftoverExpressionsForODRAfterFatalError) {
  std::vector<std::string> Args(1, "-std=c++11");
  ASSERT_EXIT({
      runToolOnCodeWithArgs(
      new TestAction,
      "namespace cb {                                                           \n"
      "/* This first error will guranatee, that we ignore the next error about  \n"
      " * uknown-type size_t.                                                   \n"
      " */                                                                      \n"
      "#include <no-such-header.h>                                              \n"
      "/**                                                                      \n"
      " * size_t -- must be not first argument                                  \n"
      " */                                                                      \n"
      "extern int callback(int, size_t);                                        \n"
      "}                                                                        \n"
      "                                                                         \n"
      "namespace with_sizet {                                                   \n"
      "                                                                         \n"
      "typedef unsigned long size_t;                                            \n"
      "/** Also it must return something */                                     \n"
      "int call(int (*__convf) (int, size_t), size_t)                           \n"
      "{ return 1; }                                                            \n"
      "                                                                         \n"
      "}                                                                        \n"
      "                                                                         \n"
      "namespace without_sizet {                                                \n"
      "                                                                         \n"
      "int to_string(float __val)                                               \n"
      "{                                                                        \n"
      "    const int n = 20; /** failed with const qualifier only */            \n"
      "    return with_sizet::call(&cb::callback, n);                           \n"
      "}                                                                        \n"
      "                                                                         \n"
      "}                                                                        \n",
      Args);
      _exit(0);
  }, ::testing::ExitedWithCode(0), ".*");
}
