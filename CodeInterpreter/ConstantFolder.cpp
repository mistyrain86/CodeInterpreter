#include "ConstantFolder.h"

std::vector<StmtPtr> ConstantFolder::optimize(std::vector<StmtPtr> stmts) {
    for (auto& s : stmts) s->accept(*this);
    return stmts;
}

ExprPtr ConstantFolder::foldExpr(ExprPtr expr) {
    auto* bin = dynamic_cast<BinaryExpr*>(expr.get());
    if (!bin) return expr;

    bin->left  = foldExpr(std::move(bin->left));
    bin->right = foldExpr(std::move(bin->right));

    auto* ll = dynamic_cast<LiteralExpr*>(bin->left.get());
    auto* rr = dynamic_cast<LiteralExpr*>(bin->right.get());

    if (ll && rr &&
        std::holds_alternative<double>(ll->value) &&
        std::holds_alternative<double>(rr->value)) {
        double a = std::get<double>(ll->value);
        double b = std::get<double>(rr->value);
        switch (bin->op.type) {
            case TokenType::PLUS:  return std::make_unique<LiteralExpr>(Value{a + b});
            case TokenType::MINUS: return std::make_unique<LiteralExpr>(Value{a - b});
            case TokenType::STAR:  return std::make_unique<LiteralExpr>(Value{a * b});
            case TokenType::SLASH:
                if (b != 0.0) return std::make_unique<LiteralExpr>(Value{a / b});
                break;
            default: break;
        }
    }
    return expr;
}

// ── StmtVisitor 구현 ───────────────────────────────────────────────

void ConstantFolder::visitExprStmt(ExprStmt& s) {
    s.expression = foldExpr(std::move(s.expression));
}
void ConstantFolder::visitPrintStmt(PrintStmt& s) {
    s.expression = foldExpr(std::move(s.expression));
}
void ConstantFolder::visitVarStmt(VarStmt& s) {
    if (s.initializer) s.initializer = foldExpr(std::move(s.initializer));
}
void ConstantFolder::visitBlockStmt(BlockStmt& s) {
    for (auto& inner : s.statements) inner->accept(*this);
}
void ConstantFolder::visitIfStmt(IfStmt& s) {
    s.condition = foldExpr(std::move(s.condition));
    s.thenBranch->accept(*this);
    if (s.elseBranch) s.elseBranch->accept(*this);
}
void ConstantFolder::visitForStmt(ForStmt& s) {
    if (s.initializer) s.initializer->accept(*this);
    if (s.condition)   s.condition  = foldExpr(std::move(s.condition));
    if (s.increment)   s.increment  = foldExpr(std::move(s.increment));
    if (s.body)        s.body->accept(*this);
}
void ConstantFolder::visitFunctionStmt(FunctionStmt& s) {
    for (auto& inner : s.body) inner->accept(*this);
}
void ConstantFolder::visitReturnStmt(ReturnStmt& s) {
    if (s.value) s.value = foldExpr(std::move(s.value));
}
