#include "ConstantFolder.h"

std::vector<StmtPtr> ConstantFolder::optimize(std::vector<StmtPtr> stmts) {
    for (auto& s : stmts) foldStmt(*s);
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

void ConstantFolder::foldStmt(Stmt& stmt) {
    if (auto* s = dynamic_cast<VarStmt*>(&stmt)) {
        if (s->initializer) s->initializer = foldExpr(std::move(s->initializer));
    } else if (auto* s = dynamic_cast<PrintStmt*>(&stmt)) {
        s->expression = foldExpr(std::move(s->expression));
    } else if (auto* s = dynamic_cast<ExprStmt*>(&stmt)) {
        s->expression = foldExpr(std::move(s->expression));
    } else if (auto* s = dynamic_cast<BlockStmt*>(&stmt)) {
        for (auto& inner : s->statements) foldStmt(*inner);
    } else if (auto* s = dynamic_cast<IfStmt*>(&stmt)) {
        s->condition = foldExpr(std::move(s->condition));
        foldStmt(*s->thenBranch);
        if (s->elseBranch) foldStmt(*s->elseBranch);
    } else if (auto* s = dynamic_cast<ForStmt*>(&stmt)) {
        if (s->initializer) foldStmt(*s->initializer);
        if (s->condition)   s->condition = foldExpr(std::move(s->condition));
        if (s->increment)   s->increment = foldExpr(std::move(s->increment));
        if (s->body)        foldStmt(*s->body);
    } else if (auto* s = dynamic_cast<FunctionStmt*>(&stmt)) {
        for (auto& inner : s->body) foldStmt(*inner);
    } else if (auto* s = dynamic_cast<ReturnStmt*>(&stmt)) {
        if (s->value) s->value = foldExpr(std::move(s->value));
    }
}
