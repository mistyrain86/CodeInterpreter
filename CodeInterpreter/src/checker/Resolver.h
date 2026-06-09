#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "BindingMap.h"
#include "Stmt.h"
#include "Expr.h"
#include "StmtVisitor.h"
#include "VoidExprVisitor.h"

class Resolver : public StmtVisitor, public VoidExprVisitor {
public:
    BindingMap resolve(const std::vector<StmtPtr>& stmts);

    void visitExprStmt    (ExprStmt&)     override;
    void visitPrintStmt   (PrintStmt&)    override;
    void visitVarStmt     (VarStmt&)      override;
    void visitBlockStmt   (BlockStmt&)    override;
    void visitIfStmt      (IfStmt&)       override;
    void visitForStmt     (ForStmt&)      override;
    void visitFunctionStmt(FunctionStmt&) override;
    void visitReturnStmt  (ReturnStmt&)   override;

    void visitGrouping    (GroupingExpr&)  override;
    void visitUnary       (UnaryExpr&)     override;
    void visitBinary      (BinaryExpr&)    override;
    void visitLogical     (LogicalExpr&)   override;
    void visitVariable    (VariableExpr&)  override;
    void visitAssign      (AssignExpr&)    override;
    void visitCallExpr    (CallExpr&)      override;
    void visitIndexGetExpr(IndexGetExpr&)  override;
    void visitIndexSetExpr(IndexSetExpr&)  override;

private:
    std::vector<std::unordered_map<std::string, bool>> m_scopes;
    BindingMap m_bindings;
    int        m_functionDepth = 0;

    void resolveStmts(const std::vector<StmtPtr>& stmts);
    void resolveLocal(Expr& expr, const std::string& name);
    void beginScope();
    void endScope();
    void declare(const Token& name);
    void define (const Token& name);
};
