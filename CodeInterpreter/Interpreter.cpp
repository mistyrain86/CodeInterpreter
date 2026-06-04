#include "Interpreter.h"
#include <cmath>
#include <iostream>
#include <sstream>

Interpreter::Interpreter()
    : m_currentEnv(std::make_shared<Environment>()) {}

void Interpreter::interpret(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) execute(s.get());
}

Value Interpreter::evaluate(Expr* expr) {
    if (auto* e = dynamic_cast<LiteralExpr*>(expr))  return e->value;
    if (auto* e = dynamic_cast<GroupingExpr*>(expr)) return evaluate(e->expression.get());
    if (auto* e = dynamic_cast<UnaryExpr*>(expr)) {
        Value r = evaluate(e->right.get());
        if (e->op.type == TokenType::MINUS) {
            if (!std::holds_alternative<double>(r))
                throw RuntimeError("[라인 " + std::to_string(e->op.line)
                    + "] 런타임 오류: 피연산자는 반드시 숫자여야 합니다.");
            return -std::get<double>(r);
        }
        if (e->op.type == TokenType::BANG) return !isTruthy(r);
    }
    throw RuntimeError("미구현 표현식 타입");
}

void Interpreter::execute(Stmt* stmt) {
    if (auto* s = dynamic_cast<PrintStmt*>(stmt))
        std::cout << stringify(evaluate(s->expression.get())) << "\n";
    else if (auto* s = dynamic_cast<ExprStmt*>(stmt))
        evaluate(s->expression.get());
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& stmts,
                                std::shared_ptr<Environment> env) {
    auto prev = m_currentEnv;
    m_currentEnv = std::move(env);
    try { for (const auto& s : stmts) execute(s.get()); }
    catch (...) { m_currentEnv = prev; throw; }
    m_currentEnv = prev;
}

bool Interpreter::isTruthy(const Value& v) const {
    if (std::holds_alternative<std::monostate>(v)) return false;
    if (std::holds_alternative<bool>(v))           return std::get<bool>(v);
    if (std::holds_alternative<double>(v))         return std::get<double>(v) != 0.0;
    return true;
}

std::string Interpreter::stringify(const Value& v) const {
    if (std::holds_alternative<std::monostate>(v)) return "nil";
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? "true" : "false";
    if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        if (std::isfinite(d) && d == std::floor(d))
            return std::to_string(static_cast<long long>(d));
        std::ostringstream oss; oss << d; return oss.str();
    }
    return std::get<std::string>(v);
}
