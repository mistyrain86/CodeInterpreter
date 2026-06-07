#pragma once
#include <map>
#include <string>
#include <vector>
#include "CheckError.h"
#include "IChecker.h"
#include "Expr.h"
#include "Stmt.h"
#include "StmtVisitor.h"
#include <unordered_set>  

class Checker : public IChecker
              , public StmtVisitor {
public:
    Checker() = default;
    void check(const std::vector<StmtPtr>& stmts) override;
    void registerGlobal(const std::string& name);

    // StmtVisitor
    void visitExprStmt    (ExprStmt&)     override;
    void visitPrintStmt   (PrintStmt&)    override;
    void visitVarStmt     (VarStmt&)      override;
    void visitBlockStmt   (BlockStmt&)    override;
    void visitIfStmt      (IfStmt&)       override;
    void visitForStmt     (ForStmt&)      override;
    void visitFunctionStmt(FunctionStmt&) override;
    void visitReturnStmt  (ReturnStmt&)   override;

private:
    std::vector<std::map<std::string, bool>> m_scopes;
    std::unordered_set<std::string>          m_knownGlobals;

    int  m_functionDepth = 0;

    void checkStmts(const std::vector<StmtPtr>& stmts);
    void checkExpr(Expr* expr);
    void beginScope();
    void endScope();
    void declare(const Token& name);
    void define(const Token& name);
    void resolveVar(const std::string& name, int line);
};
