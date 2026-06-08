#include "Resolver.h"
#include <stdexcept>

BindingMap Resolver::resolve(const std::vector<StmtPtr>& stmts) {
    m_bindings.clear();
    m_scopes.clear();
    m_functionDepth = 0;
    resolveStmts(stmts);
    return m_bindings;
}

void Resolver::resolveStmts(const std::vector<StmtPtr>& stmts) {
    for (auto& s : stmts) resolveStmt(*s);
}

void Resolver::resolveStmt(Stmt& stmt) {
    if (auto* s = dynamic_cast<VarStmt*>(&stmt)) {
        declare(s->name);
        if (s->initializer) resolveExpr(*s->initializer);
        define(s->name);
    }
    else if (auto* s = dynamic_cast<FunctionStmt*>(&stmt)) {
        declare(s->name); define(s->name);
        m_functionDepth++;
        beginScope();
        for (auto& p : s->params) { declare(p); define(p); }
        resolveStmts(s->body);
        endScope();
        m_functionDepth--;
    }
    else if (auto* s = dynamic_cast<ReturnStmt*>(&stmt)) {
        if (s->value) resolveExpr(*s->value);
    }
    else if (auto* s = dynamic_cast<BlockStmt*>(&stmt)) {
        beginScope(); resolveStmts(s->statements); endScope();
    }
    else if (auto* s = dynamic_cast<IfStmt*>(&stmt)) {
        resolveExpr(*s->condition);
        resolveStmt(*s->thenBranch);
        if (s->elseBranch) resolveStmt(*s->elseBranch);
    }
    else if (auto* s = dynamic_cast<ForStmt*>(&stmt)) {
        beginScope();
        if (s->initializer) resolveStmt(*s->initializer);
        if (s->condition)   resolveExpr(*s->condition);
        if (s->increment)   resolveExpr(*s->increment);
        if (s->body)        resolveStmt(*s->body);
        endScope();
    }
    else if (auto* s = dynamic_cast<PrintStmt*>(&stmt)) {
        resolveExpr(*s->expression);
    }
    else if (auto* s = dynamic_cast<ExprStmt*>(&stmt)) {
        resolveExpr(*s->expression);
    }
}

void Resolver::resolveExpr(Expr& expr) {
    if (auto* e = dynamic_cast<VariableExpr*>(&expr)) {
        resolveLocal(expr, e->name.lexeme);
    }
    else if (auto* e = dynamic_cast<AssignExpr*>(&expr)) {
        resolveExpr(*e->value);
        resolveLocal(expr, e->name.lexeme);
    }
    else if (auto* e = dynamic_cast<BinaryExpr*>(&expr)) {
        resolveExpr(*e->left); resolveExpr(*e->right);
    }
    else if (auto* e = dynamic_cast<UnaryExpr*>(&expr)) {
        resolveExpr(*e->right);
    }
    else if (auto* e = dynamic_cast<GroupingExpr*>(&expr)) {
        resolveExpr(*e->expression);
    }
    else if (auto* e = dynamic_cast<CallExpr*>(&expr)) {
        resolveExpr(*e->callee);
        for (auto& arg : e->args) resolveExpr(*arg);
    }
    else if (auto* e = dynamic_cast<IndexGetExpr*>(&expr)) {
        resolveExpr(*e->object); resolveExpr(*e->index);
    }
    else if (auto* e = dynamic_cast<IndexSetExpr*>(&expr)) {
        resolveExpr(*e->object);
        resolveExpr(*e->index);
        resolveExpr(*e->value);
    }
    // LiteralExpr: no-op
}

void Resolver::resolveLocal(Expr& expr, const std::string& name) {
    for (int i = (int)m_scopes.size() - 1; i >= 0; i--) {
        if (m_scopes[i].count(name)) {
            // distance = 현재 스코프 depth - 변수가 선언된 스코프 depth
            m_bindings[&expr] = (int)m_scopes.size() - 1 - i;
            return;
        }
    }
    // 전역 변수: BindingMap에 기록 안 함 → Interpreter가 env.get() 폴백
}

void Resolver::beginScope() { m_scopes.emplace_back(); }
void Resolver::endScope() { m_scopes.pop_back(); }

void Resolver::declare(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = false;
}
void Resolver::define(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = true;
}
