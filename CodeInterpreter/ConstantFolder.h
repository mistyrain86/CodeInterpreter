#pragma once
#include "IOptimizer.h"
#include "Expr.h"
#include "StmtVisitor.h"

// StmtVisitor 구현으로 새 Stmt 노드 추가 시 컴파일러가 누락을 감지
class ConstantFolder : public IOptimizer
                     , public StmtVisitor {
public:
    std::vector<StmtPtr> optimize(std::vector<StmtPtr> stmts) override;

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
    ExprPtr foldExpr(ExprPtr expr);
};
