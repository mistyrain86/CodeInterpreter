#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "IInterpreter.h"
#include "Environment.h"
#include "Expr.h"
#include "Stmt.h"
#include "Value.h"

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};

class Interpreter : public IInterpreter {
public:
    Interpreter();
    void interpret(const std::vector<StmtPtr>& stmts) override;

private:
    std::shared_ptr<Environment> m_currentEnv;
    Value       evaluate(Expr* expr);
    void        execute(Stmt* stmt);
    void        executeBlock(const std::vector<StmtPtr>& stmts,
                             std::shared_ptr<Environment> env);
    bool        isTruthy(const Value& val) const;
    std::string stringify(const Value& val) const;
};
