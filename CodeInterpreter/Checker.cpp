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
}

void Checker::checkExpr(Expr*) {  }

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
void Checker::resolveVar(const std::string&, int) {  }