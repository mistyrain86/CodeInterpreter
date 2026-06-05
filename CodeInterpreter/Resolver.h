#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "BindingMap.h"
#include "Stmt.h"
#include "Expr.h"

// Ch.4 정적 바인딩 — AST를 순회하며 변수 distance 계산
// C가 Resolver.cpp 에서 구현
class Resolver {
public:
    // stmts를 순회하여 BindingMap 반환
    BindingMap resolve(const std::vector<StmtPtr>& stmts);

private:
    // 스코프 스택: 변수명 → 초기화 여부
    std::vector<std::unordered_map<std::string, bool>> m_scopes;
    BindingMap m_bindings;
    int        m_functionDepth = 0;

    void resolveStmts(const std::vector<StmtPtr>& stmts);
    void resolveStmt (Stmt& stmt);
    void resolveExpr (Expr& expr);
    void resolveLocal(Expr& expr, const std::string& name);

    void beginScope();
    void endScope();
    void declare(const Token& name);
    void define (const Token& name);
};
