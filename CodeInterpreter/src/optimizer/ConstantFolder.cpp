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
    s.m_expression = foldExpr(std::move(s.m_expression));
}
void ConstantFolder::visitPrintStmt(PrintStmt& s) {
    s.m_expression = foldExpr(std::move(s.m_expression));
}
void ConstantFolder::visitVarStmt(VarStmt& s) {
    if (s.m_initializer) s.m_initializer = foldExpr(std::move(s.m_initializer));
}
void ConstantFolder::visitBlockStmt(BlockStmt& s) {
    for (auto& inner : s.m_statements) inner->accept(*this);
}
void ConstantFolder::visitIfStmt(IfStmt& s) {
    s.m_condition = foldExpr(std::move(s.m_condition));
    s.m_thenBranch->accept(*this);
    if (s.m_elseBranch) s.m_elseBranch->accept(*this);
}
void ConstantFolder::visitForStmt(ForStmt& s) {
    if (s.m_initializer) s.m_initializer->accept(*this);
    if (s.m_condition)   s.m_condition  = foldExpr(std::move(s.m_condition));
    if (s.m_increment)   s.m_increment  = foldExpr(std::move(s.m_increment));
    if (s.m_body)        s.m_body->accept(*this);
}
void ConstantFolder::visitFunctionStmt(FunctionStmt& s) {
    for (auto& inner : s.m_body) inner->accept(*this);
}
void ConstantFolder::visitReturnStmt(ReturnStmt& s) {
    if (s.m_value) s.m_value = foldExpr(std::move(s.m_value));
}
