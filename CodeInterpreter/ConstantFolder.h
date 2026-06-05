#pragma once
#include "IOptimizer.h"
#include "Expr.h"

// Ch.4 상수 폴딩 — IOptimizer 구현체
// D가 ConstantFolder.cpp 에서 구현
class ConstantFolder : public IOptimizer {
public:
    std::vector<StmtPtr> optimize(std::vector<StmtPtr> stmts) override;

private:
    // 표현식을 재귀적으로 폴딩
    // BinaryExpr(Literal, op, Literal) → LiteralExpr 교체
    ExprPtr foldExpr(ExprPtr expr);
    void    foldStmt(Stmt& stmt);
};
