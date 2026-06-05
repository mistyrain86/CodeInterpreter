#include "ConstantFolder.h"

std::vector<StmtPtr> ConstantFolder::optimize(std::vector<StmtPtr> stmts) {
    return stmts;
}

ExprPtr ConstantFolder::foldExpr(ExprPtr expr) {
    return expr;
}

void ConstantFolder::foldStmt(Stmt& stmt) {
}
