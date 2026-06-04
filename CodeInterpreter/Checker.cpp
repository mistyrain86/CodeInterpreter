#include <cassert>
#include "Checker.h"

void Checker::check(const std::vector<StmtPtr>& stmts) { checkStmts(stmts); }

void Checker::checkStmts(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) checkStmt(s.get());
}

void Checker::checkStmt(Stmt* stmt) {
    if (auto* s = dynamic_cast<VarStmt*>(stmt)) {
        declare(s->name);
        if (s->initializer) checkExpr(s->initializer.get());
        define(s->name);
    }
    else if (auto* s = dynamic_cast<BlockStmt*>(stmt)) {
        beginScope();
        checkStmts(s->statements);
        endScope();
    }
    else if (auto* s = dynamic_cast<IfStmt*>(stmt)) {
        checkExpr(s->condition.get());
        checkStmt(s->thenBranch.get());
        if (s->elseBranch) checkStmt(s->elseBranch.get());
    }
    else if (auto* s = dynamic_cast<ForStmt*>(stmt)) {
        beginScope();
        if (s->initializer) checkStmt(s->initializer.get());
        if (s->condition)   checkExpr(s->condition.get());
        if (s->increment)   checkExpr(s->increment.get());
        checkStmt(s->body.get());
        endScope();
    }
    else if (auto* s = dynamic_cast<PrintStmt*>(stmt)) {
        checkExpr(s->expression.get());
    }
    else if (auto* s = dynamic_cast<ExprStmt*>(stmt)) {
        checkExpr(s->expression.get());
    }
}

void Checker::checkExpr(Expr* expr) {
    assert(expr != nullptr);
    if (auto* e = dynamic_cast<BinaryExpr*>(expr)) {
        checkExpr(e->left.get()); checkExpr(e->right.get());
    }
    else if (auto* e = dynamic_cast<GroupingExpr*>(expr)) {
        checkExpr(e->expression.get());
    }
    else if (auto* e = dynamic_cast<UnaryExpr*>(expr)) {
        checkExpr(e->right.get());
    }
    else if (auto* e = dynamic_cast<VariableExpr*>(expr)) {
        resolveVar(e->name.lexeme, e->name.line);
    }
    else if (auto* e = dynamic_cast<AssignExpr*>(expr)) {
        checkExpr(e->value.get());
        resolveVar(e->name.lexeme, e->name.line);
    }
    // LiteralExpr: 검사 없음
}

void Checker::beginScope() { m_scopes.emplace_back(); }
void Checker::endScope() { m_scopes.pop_back(); }

void Checker::declare(const Token& name) {
    if (m_scopes.empty()) return;           // ← 전역은 검사 안 함
    auto& scope = m_scopes.back();
    if (scope.count(name.lexeme))
        throw CheckError("[라인 " + std::to_string(name.line)
            + "] 의미 오류: 이미 이 스코프에 같은 이름의 변수가 있습니다. ('"
            + name.lexeme + "')");
    scope[name.lexeme] = false;
}

void Checker::define(const Token& name) {
    if (!m_scopes.empty()) m_scopes.back()[name.lexeme] = true;
}

void Checker::resolveVar(const std::string& name, int line) {
    for (int i = (int)m_scopes.size() - 1; i >= 0; i--) {
        auto it = m_scopes[i].find(name);
        if (it != m_scopes[i].end()) {
            if (!it->second)
                throw CheckError("[라인 " + std::to_string(line)
                    + "] 의미 오류: 자신의 초기화식에서 지역변수를 읽을 수 없습니다. ('"
                    + name + "')");
            return;
        }
    }
}