#include "ConstantFolder.h"

// TODO (D): 아래 메서드들을 구현하세요.

std::vector<StmtPtr> ConstantFolder::optimize(std::vector<StmtPtr> stmts) {
    // TODO: 각 stmt를 순회하며 foldStmt 호출
    return stmts;  // 현재는 no-op (스텁)
}

ExprPtr ConstantFolder::foldExpr(ExprPtr expr) {
    // TODO: BinaryExpr(LiteralExpr op LiteralExpr) → LiteralExpr 교체
    return expr;
}

void ConstantFolder::foldStmt(Stmt& stmt) {
    // TODO: VarStmt, PrintStmt, ExprStmt 등의 expression 폴딩
}
