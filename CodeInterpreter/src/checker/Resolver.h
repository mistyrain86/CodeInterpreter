#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "BindingMap.h"
#include "Stmt.h"
#include "Expr.h"

class Resolver {
public:
    BindingMap resolve(const std::vector<StmtPtr>& stmts);

private:
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
