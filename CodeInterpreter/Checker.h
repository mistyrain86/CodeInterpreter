#pragma once
#include <map>
#include <string>
#include <vector>
#include "CheckError.h"
#include "IChecker.h"
#include "Expr.h"
#include "Stmt.h"

class Checker : public IChecker {
public:
    Checker() = default;
    void check(const std::vector<StmtPtr>& stmts) override;

private:
    std::vector<std::map<std::string, bool>> m_scopes;

    void checkStmts(const std::vector<StmtPtr>& stmts);
    void checkStmt(Stmt* stmt);
    void checkExpr(Expr* expr);
    void beginScope();
    void endScope();
    void declare(const Token& name);
    void define(const Token& name);
    void resolveVar(const std::string& name, int line);
};