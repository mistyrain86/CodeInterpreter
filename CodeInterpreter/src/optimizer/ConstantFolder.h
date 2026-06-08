#pragma once
#include "IOptimizer.h"
#include "Expr.h"
#include "StmtVisitor.h"

class ConstantFolder : public IOptimizer
                     , public StmtVisitor {
public:
    std::vector<StmtPtr> optimize(std::vector<StmtPtr> stmts) override;

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
