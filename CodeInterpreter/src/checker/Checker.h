#pragma once
#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include "CheckError.h"
#include "IChecker.h"
#include "Expr.h"
#include "Stmt.h"
#include "StmtVisitor.h"
#include "VoidExprVisitor.h"

class Checker : public IChecker
              , public StmtVisitor
              , public VoidExprVisitor {
public:
    Checker() = default;
    void check(const std::vector<StmtPtr>& stmts) override;
    void registerGlobal(const std::string& name) override;

    // StmtVisitor
    void visitExprStmt    (ExprStmt&)     override;
    void visitPrintStmt   (PrintStmt&)    override;
    void visitVarStmt     (VarStmt&)      override;
    void visitBlockStmt   (BlockStmt&)    override;
    void visitIfStmt      (IfStmt&)       override;
    void visitForStmt     (ForStmt&)      override;
    void visitFunctionStmt(FunctionStmt&) override;
    void visitReturnStmt  (ReturnStmt&)   override;

    // VoidExprVisitor
    void visitGrouping    (GroupingExpr&)  override;
    void visitUnary       (UnaryExpr&)     override;
    void visitBinary      (BinaryExpr&)    override;
    void visitVariable    (VariableExpr&)  override;
    void visitAssign      (AssignExpr&)    override;
    void visitCallExpr    (CallExpr&)      override;
    void visitIndexGetExpr(IndexGetExpr&)  override;
    void visitIndexSetExpr(IndexSetExpr&)  override;

private:
    std::vector<std::map<std::string, bool>> m_scopes;
    std::unordered_set<std::string>          m_knownGlobals;
    int  m_functionDepth = 0;
    bool m_inUserCode    = false;  // 사용자 코드 검사 중 여부 (전역 재선언 검사용)

    void checkStmts(const std::vector<StmtPtr>& stmts);
    void beginScope();
    void endScope();
    void declare(const Token& name);
    void define(const Token& name);
    void resolveVar(const std::string& name, int line);
};
