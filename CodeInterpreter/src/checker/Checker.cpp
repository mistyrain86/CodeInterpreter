#include <cassert>
#include "Checker.h"

void Checker::check(const std::vector<StmtPtr>& stmts) {
    beginScope();
    for (const auto& name : m_knownGlobals)
        m_scopes.back()[name] = true;
    beginScope();

    Token arrayBuiltin{TokenType::IDENTIFIER, "Array", std::monostate{}, 0};
    declare(arrayBuiltin);
    define(arrayBuiltin);
    checkStmts(stmts);

    endScope();
    endScope();
}

void Checker::registerGlobal(const std::string& name) {
    m_knownGlobals.insert(name);
}

void Checker::checkStmts(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) s->accept(*this);
}

// ── StmtVisitor 구현 ───────────────────────────────────────────────

void Checker::visitVarStmt(VarStmt& s) {
    declare(s.m_name);
    if (s.m_initializer) checkExpr(s.m_initializer.get());
    define(s.m_name);
}

void Checker::visitBlockStmt(BlockStmt& s) {
    beginScope();
    checkStmts(s.m_statements);
    endScope();
}

void Checker::visitIfStmt(IfStmt& s) {
    checkExpr(s.m_condition.get());
    s.m_thenBranch->accept(*this);
    if (s.m_elseBranch) s.m_elseBranch->accept(*this);
}

void Checker::visitForStmt(ForStmt& s) {
    beginScope();
    if (s.m_initializer) s.m_initializer->accept(*this);
    if (s.m_condition)   checkExpr(s.m_condition.get());
    if (s.m_increment)   checkExpr(s.m_increment.get());
    if (s.m_body)        s.m_body->accept(*this);
    endScope();
}

void Checker::visitPrintStmt(PrintStmt& s) {
    checkExpr(s.m_expression.get());
}

void Checker::visitExprStmt(ExprStmt& s) {
    checkExpr(s.m_expression.get());
}

void Checker::visitFunctionStmt(FunctionStmt& s) {
    declare(s.m_name);
    define(s.m_name);


    std::unordered_set<std::string> seen;
    for (const auto& param : s.m_params) {
        if (seen.count(param.lexeme))
            throw CheckError("[라인 " + std::to_string(param.line)
                + "] 의미 오류: 파라미터 이름이 중복됩니다. ('"
                + param.lexeme + "')");
        seen.insert(param.lexeme);
    }

    m_functionDepth++;
    beginScope();
    for (const auto& param : s.m_params) {
        declare(param);
        define(param);
    }
    checkStmts(s.m_body);
    endScope();
    m_functionDepth--;
}

void Checker::visitReturnStmt(ReturnStmt& s) {
    if (m_functionDepth == 0)
        throw CheckError("[라인 " + std::to_string(s.m_keyword.line)
            + "] 의미 오류: 함수 외부에서 return을 사용할 수 없습니다.");
    if (s.m_value) checkExpr(s.m_value.get());
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
        resolveVar(e->name.lexeme, e->name.line);
    }
    else if (auto* e = dynamic_cast<CallExpr*>(expr)) {
        checkExpr(e->callee.get());
        for (auto& arg : e->args) checkExpr(arg.get());
    }
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
    auto& scope = m_scopes.back();
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
