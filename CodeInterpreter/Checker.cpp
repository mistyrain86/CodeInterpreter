#include <cassert>
#include "Checker.h"

void Checker::check(const std::vector<StmtPtr>& stmts) {
    beginScope();
    // 내장 함수 등록
    Token arrayBuiltin{TokenType::IDENTIFIER, "Array", std::monostate{}, 0};
    declare(arrayBuiltin);
    define(arrayBuiltin);
    checkStmts(stmts);
    endScope();
}

void Checker::checkStmts(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) s->accept(*this);
}

// ── StmtVisitor 구현 ───────────────────────────────────────────────

void Checker::visitVarStmt(VarStmt& s) {
    declare(s.name);
    if (s.initializer) checkExpr(s.initializer.get());
    define(s.name);
}

void Checker::visitBlockStmt(BlockStmt& s) {
    beginScope();
    checkStmts(s.statements);
    endScope();
}

void Checker::visitIfStmt(IfStmt& s) {
    checkExpr(s.condition.get());
    s.thenBranch->accept(*this);
    if (s.elseBranch) s.elseBranch->accept(*this);
}

void Checker::visitForStmt(ForStmt& s) {
    beginScope();
    if (s.initializer) s.initializer->accept(*this);
    if (s.condition)   checkExpr(s.condition.get());
    if (s.increment)   checkExpr(s.increment.get());
    if (s.body)        s.body->accept(*this);
    endScope();
}

void Checker::visitPrintStmt(PrintStmt& s) {
    checkExpr(s.expression.get());
}

void Checker::visitExprStmt(ExprStmt& s) {
    checkExpr(s.expression.get());
}

// ── Ch.2 함수 스텁 (C가 구현) ─────────────────────────────────────
void Checker::visitFunctionStmt(FunctionStmt& s) {
    // 함수 이름을 외부 스코프에 등록 (호출 위치에서 resolveVar 가능)
    declare(s.name);
    define(s.name);

    // 파라미터 이름 중복 검사
    std::unordered_set<std::string> seen;
    for (const auto& param : s.params) {
        if (seen.count(param.lexeme))
            throw CheckError("[라인 " + std::to_string(param.line)
                + "] 의미 오류: 파라미터 이름이 중복됩니다. ('"
                + param.lexeme + "')");
        seen.insert(param.lexeme);
    }
    // 함수 본문 스코프 검사
    m_functionDepth++;
    beginScope();
    for (const auto& param : s.params) {
        declare(param);
        define(param);
    }
    checkStmts(s.body);
    endScope();
    m_functionDepth--;
}

void Checker::visitReturnStmt(ReturnStmt& s) {
    if (m_functionDepth == 0)
        throw CheckError("[라인 " + std::to_string(s.keyword.line)
            + "] 의미 오류: 함수 외부에서 return을 사용할 수 없습니다.");
    if (s.value) checkExpr(s.value.get());
}

// ── 표현식 분석 (dynamic_cast 유지 — void 반환) ──────────────────

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
        // 스코프 체인에 없으면 전역에 암묵적 선언 (미선언 변수 할당 허용)
        bool found = false;
        for (int i = (int)m_scopes.size() - 1; i >= 0; i--) {
            if (m_scopes[i].count(e->name.lexeme)) { found = true; break; }
        }
        if (!found) m_scopes.front()[e->name.lexeme] = true;
    }
    // Ch.2: 함수 호출 — 인자 표현식 재귀 검사
    else if (auto* e = dynamic_cast<CallExpr*>(expr)) {
        checkExpr(e->callee.get());
        for (auto& arg : e->args) checkExpr(arg.get());
    }
    // Ch.3: 배열 인덱스
    else if (auto* e = dynamic_cast<IndexGetExpr*>(expr)) {
        checkExpr(e->object.get());
        checkExpr(e->index.get());
    }
    else if (auto* e = dynamic_cast<IndexSetExpr*>(expr)) {
        checkExpr(e->object.get());
        checkExpr(e->index.get());
        checkExpr(e->value.get());
    }
    // LiteralExpr: 검사 없음
}

// ── 스코프 관리 ────────────────────────────────────────────────────

void Checker::beginScope() { m_scopes.emplace_back(); }
void Checker::endScope()   { m_scopes.pop_back(); }

void Checker::declare(const Token& name) {
    auto& scope = m_scopes.back();      // 전역 포함 모든 스코프 검사
    if (scope.count(name.lexeme))
        throw CheckError("[라인 " + std::to_string(name.line)
            + "] 의미 오류: 이미 이 스코프에 같은 이름의 변수가 있습니다. ('"
            + name.lexeme + "')");
    scope[name.lexeme] = false;
}

void Checker::define(const Token& name) {
    m_scopes.back()[name.lexeme] = true;
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
    throw CheckError("[라인 " + std::to_string(line)
        + "] 의미 오류: 선언되지 않은 변수입니다. ('" + name + "')");
}
