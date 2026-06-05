#include "Resolver.h"

// TODO (C): 아래 메서드들을 구현하세요.

BindingMap Resolver::resolve(const std::vector<StmtPtr>& stmts) {
    m_bindings.clear();
    m_scopes.clear();
    resolveStmts(stmts);
    return m_bindings;
}

void Resolver::resolveStmts(const std::vector<StmtPtr>& stmts) {
    for (auto& s : stmts) resolveStmt(*s);
}

void Resolver::resolveStmt(Stmt& stmt) {
    // TODO: dynamic_cast 분기로 각 Stmt 처리
    // VarStmt, BlockStmt, IfStmt, ForStmt, FunctionStmt, ReturnStmt,
    // PrintStmt, ExprStmt
}

void Resolver::resolveExpr(Expr& expr) {
    // TODO: dynamic_cast 분기로 각 Expr 처리
    // VariableExpr → resolveLocal 호출
    // AssignExpr   → resolveLocal 호출
    // CallExpr, BinaryExpr, UnaryExpr, GroupingExpr, IndexGetExpr, IndexSetExpr
}

void Resolver::resolveLocal(Expr& expr, const std::string& name) {
    // TODO: m_scopes를 역순으로 탐색, 발견 시 distance 계산
    // m_bindings[&expr] = distance;
}

void Resolver::beginScope() { m_scopes.emplace_back(); }
void Resolver::endScope()   { m_scopes.pop_back(); }

void Resolver::declare(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = false;
}
void Resolver::define(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = true;
}
