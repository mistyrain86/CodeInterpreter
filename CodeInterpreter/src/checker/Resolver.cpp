#include "Resolver.h"

BindingMap Resolver::resolve(const std::vector<StmtPtr>& stmts) {
    m_bindings.clear();
    m_scopes.clear();
    m_functionDepth = 0;
    resolveStmts(stmts);
    return m_bindings;
}

void Resolver::resolveStmts(const std::vector<StmtPtr>& stmts) {
    for (auto& s : stmts) s->accept(*this);
}

// ── StmtVisitor 구현 ───────────────────────────────────────────────

void Resolver::visitVarStmt(VarStmt& s) {
    declare(s.m_name);
    if (s.m_initializer) s.m_initializer->acceptVoid(*this);
    define(s.m_name);
}

void Resolver::visitFunctionStmt(FunctionStmt& s) {
    declare(s.m_name); define(s.m_name);
    m_functionDepth++;
    beginScope();
    for (auto& p : s.m_params) { declare(p); define(p); }
    resolveStmts(s.m_body);
    endScope();
    m_functionDepth--;
}

void Resolver::visitReturnStmt(ReturnStmt& s) {
    if (s.m_value) s.m_value->acceptVoid(*this);
}

void Resolver::visitBlockStmt(BlockStmt& s) {
    beginScope(); resolveStmts(s.m_statements); endScope();
}

void Resolver::visitIfStmt(IfStmt& s) {
    s.m_condition->acceptVoid(*this);
    s.m_thenBranch->accept(*this);
    if (s.m_elseBranch) s.m_elseBranch->accept(*this);
}

void Resolver::visitForStmt(ForStmt& s) {
    beginScope();
    if (s.m_initializer) s.m_initializer->accept(*this);
    if (s.m_condition)   s.m_condition->acceptVoid(*this);
    if (s.m_increment)   s.m_increment->acceptVoid(*this);
    if (s.m_body)        s.m_body->accept(*this);
    endScope();
}

void Resolver::visitPrintStmt(PrintStmt& s) {
    s.m_expression->acceptVoid(*this);
}

void Resolver::visitExprStmt(ExprStmt& s) {
    s.m_expression->acceptVoid(*this);
}

// ── VoidExprVisitor 구현 ───────────────────────────────────────────

void Resolver::visitVariable(VariableExpr& e) {
    resolveLocal(e, e.name.lexeme);
}

void Resolver::visitAssign(AssignExpr& e) {
    e.value->acceptVoid(*this);
    resolveLocal(e, e.name.lexeme);
}

void Resolver::visitBinary(BinaryExpr& e) {
    e.left->acceptVoid(*this); e.right->acceptVoid(*this);
}

void Resolver::visitUnary(UnaryExpr& e) {
    e.right->acceptVoid(*this);
}

void Resolver::visitGrouping(GroupingExpr& e) {
    e.expression->acceptVoid(*this);
}

void Resolver::visitCallExpr(CallExpr& e) {
    e.callee->acceptVoid(*this);
    for (auto& arg : e.args) arg->acceptVoid(*this);
}

void Resolver::visitIndexGetExpr(IndexGetExpr& e) {
    e.object->acceptVoid(*this); e.index->acceptVoid(*this);
}

void Resolver::visitIndexSetExpr(IndexSetExpr& e) {
    e.object->acceptVoid(*this);
    e.index->acceptVoid(*this);
    e.value->acceptVoid(*this);
}

// ── 스코프 관리 ────────────────────────────────────────────────────

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
void Resolver::endScope()   { m_scopes.pop_back(); }

void Resolver::declare(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = false;
}
void Resolver::define(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = true;
}
