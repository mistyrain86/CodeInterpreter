#include "ConstantFolder.h"
#include <cmath>

ConstantFolder::ConstantFolder() { initFoldOps(); }

void ConstantFolder::initFoldOps() {
    using T = TokenType;
    m_foldOps[static_cast<int>(T::PLUS)]  = [](double a, double b) -> std::optional<double> { return a + b; };
    m_foldOps[static_cast<int>(T::MINUS)] = [](double a, double b) -> std::optional<double> { return a - b; };
    m_foldOps[static_cast<int>(T::STAR)]  = [](double a, double b) -> std::optional<double> { return a * b; };
    m_foldOps[static_cast<int>(T::SLASH)] = [](double a, double b) -> std::optional<double> {
        if (b == 0.0) return std::nullopt;  // 0 나누기는 런타임에 위임
        return a / b;
    };
    m_foldOps[static_cast<int>(T::PERCENT)] = [](double a, double b) -> std::optional<double> {
        if (b == 0.0) return std::nullopt;
        return std::fmod(a, b);
    };
}

std::vector<StmtPtr> ConstantFolder::optimize(std::vector<StmtPtr> stmts) {
    for (auto& s : stmts) s->accept(*this);
    return stmts;
}

ExprPtr ConstantFolder::foldExpr(ExprPtr expr) {
    // GroupingExpr: 내부를 폴딩 후 결과가 LiteralExpr이면 GroupingExpr 제거
    if (auto* group = dynamic_cast<GroupingExpr*>(expr.get())) {
        group->expression = foldExpr(std::move(group->expression));
        if (dynamic_cast<LiteralExpr*>(group->expression.get()))
            return std::move(group->expression);  // GroupingExpr 벗겨냄
        return expr;
    }

    // AssignExpr: 우변만 폴딩 (좌변은 변수명이므로 건드리지 않음)
    if (auto* assign = dynamic_cast<AssignExpr*>(expr.get())) {
        assign->value = foldExpr(std::move(assign->value));
        return expr;
    }

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
        auto it = m_foldOps.find(static_cast<int>(bin->op.type));
        if (it != m_foldOps.end()) {
            auto result = it->second(a, b);
            if (result) return std::make_unique<LiteralExpr>(Value{*result});
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
