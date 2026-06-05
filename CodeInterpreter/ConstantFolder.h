#pragma once
#include "IOptimizer.h"
#include "Expr.h"

class ConstantFolder : public IOptimizer {
public:
    std::vector<StmtPtr> optimize(std::vector<StmtPtr> stmts) override;

private:
    ExprPtr foldExpr(ExprPtr expr);
    void    foldStmt(Stmt& stmt);
};
