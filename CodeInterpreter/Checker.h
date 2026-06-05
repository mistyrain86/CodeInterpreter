#pragma once
#include <map>
#include <string>
#include <vector>
#include "CheckError.h"
#include "IChecker.h"
#include "Expr.h"
#include "Stmt.h"
#include "StmtVisitor.h"

class Checker : public IChecker
              , public StmtVisitor {
public:
    Checker() = default;
    void check(const std::vector<StmtPtr>& stmts) override;

    // StmtVisitor
    void visitExprStmt    (ExprStmt&)     override;
    void visitPrintStmt   (PrintStmt&)    override;
    void visitVarStmt     (VarStmt&)      override;
    void visitBlockStmt   (BlockStmt&)    override;
    void visitIfStmt      (IfStmt&)       override;
    void visitForStmt     (ForStmt&)      override;
    void visitFunctionStmt(FunctionStmt&) override;  // Ch.2 — C 구현
    void visitReturnStmt  (ReturnStmt&)   override;  // Ch.2 — C 구현

private:
    std::vector<std::map<std::string, bool>> m_scopes;

    int  m_functionDepth = 0;     // Ch.2 return 위치 검증용

    void checkStmts(const std::vector<StmtPtr>& stmts);
    void checkExpr(Expr* expr);   // 표현식은 dynamic_cast 유지 (void 반환)
    void beginScope();
    void endScope();
    void declare(const Token& name);
    void define(const Token& name);
    void resolveVar(const std::string& name, int line);
};
