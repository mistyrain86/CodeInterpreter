#include "Interpreter.h"
#include <cmath>
#include <iostream>
#include <sstream>

namespace {
static constexpr auto UNIMPLEMENTED_EXPR   = "미구현 표현식 타입";
static constexpr auto UNIMPLEMENTED_UNARY  = "미구현 단항 연산자";
static constexpr auto UNIMPLEMENTED_BINARY = "미구현 이항 연산자";

struct ScopeGuard {
    std::shared_ptr<Environment>& ref;
    std::shared_ptr<Environment>  prev;
    ScopeGuard(std::shared_ptr<Environment>& r, std::shared_ptr<Environment> next)
        : ref(r), prev(r) { r = std::move(next); }
    ~ScopeGuard() { ref = prev; }
};
}

Interpreter::Interpreter()
    : m_currentEnv(std::make_shared<Environment>()) {}

// ── 공개 진입점 ────────────────────────────────────────────────────

void Interpreter::interpret(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) execute(*s);
}

Value Interpreter::evaluate(Expr& expr) {
    return expr.accept(*this);
}

void Interpreter::execute(Stmt& stmt) {
    stmt.accept(*this);
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& stmts,
                                std::shared_ptr<Environment> env) {
    ScopeGuard guard(m_currentEnv, std::move(env));
    for (const auto& s : stmts) execute(*s);
}

// ── ExprVisitor 구현 ───────────────────────────────────────────────

Value Interpreter::visitLiteral(LiteralExpr& e) {
    return e.value;
}

Value Interpreter::visitGrouping(GroupingExpr& e) {
    return evaluate(*e.expression);
}

Value Interpreter::visitUnary(UnaryExpr& e) {
    Value r = evaluate(*e.right);
    if (e.op.type == TokenType::MINUS) {
        checkNumericOperand(r, e.op.line);
        return -std::get<double>(r);
    }
    if (e.op.type == TokenType::BANG) return !isTruthy(r);
    throw RuntimeError(UNIMPLEMENTED_UNARY);
}

Value Interpreter::visitBinary(BinaryExpr& e) {
    Value l = evaluate(*e.left);
    Value r = evaluate(*e.right);
    const int line = e.op.line;
    switch (e.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
                return std::get<double>(l) + std::get<double>(r);
            if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
                return std::get<std::string>(l) + std::get<std::string>(r);
            throw RuntimeError("[라인 " + std::to_string(line)
                + "] 런타임 오류: 피연산자는 두 숫자 또는 두 문자열이어야 합니다.");
        case TokenType::MINUS: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            return dl - dr;
        }
        case TokenType::STAR: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            return dl * dr;
        }
        case TokenType::SLASH: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            if (dr == 0.0)
                throw RuntimeError("[라인 " + std::to_string(line)
                    + "] 런타임 오류: 0으로 나눌 수 없습니다.");
            return dl / dr;
        }
        case TokenType::GREATER: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            return dl > dr;
        }
        case TokenType::GREATER_EQUAL: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            return dl >= dr;
        }
        case TokenType::LESS: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            return dl < dr;
        }
        case TokenType::LESS_EQUAL: {
            checkNumericPair(l, r, line);
            const double dl = std::get<double>(l), dr = std::get<double>(r);
            return dl <= dr;
        }
        case TokenType::EQUAL_EQUAL: return Value{l == r};
        case TokenType::BANG_EQUAL:  return Value{!(l == r)};
        default: break;
    }
    throw RuntimeError(UNIMPLEMENTED_BINARY);
}

Value Interpreter::visitVariable(VariableExpr& e) {
    return m_currentEnv->get(e.name);
}

Value Interpreter::visitAssign(AssignExpr& e) {
    Value v = evaluate(*e.value);
    m_currentEnv->assign(e.name, v);
    return v;
}

// ── StmtVisitor 구현 ───────────────────────────────────────────────

void Interpreter::visitExprStmt(ExprStmt& s) {
    evaluate(*s.expression);
}

void Interpreter::visitPrintStmt(PrintStmt& s) {
    std::cout << stringify(evaluate(*s.expression)) << "\n";
}

void Interpreter::visitVarStmt(VarStmt& s) {
    Value v = s.initializer ? evaluate(*s.initializer) : Value{std::monostate{}};
    m_currentEnv->define(s.name.lexeme, std::move(v));
}

void Interpreter::visitBlockStmt(BlockStmt& s) {
    executeBlock(s.statements, std::make_shared<Environment>(m_currentEnv));
}

void Interpreter::visitIfStmt(IfStmt& s) {
    if (isTruthy(evaluate(*s.condition))) execute(*s.thenBranch);
    else if (s.elseBranch)               execute(*s.elseBranch);
}

void Interpreter::visitForStmt(ForStmt& s) {
    if (!s.body)
        throw RuntimeError("런타임 오류: ForStmt body가 null입니다.");
    ScopeGuard guard(m_currentEnv, std::make_shared<Environment>(m_currentEnv));
    if (s.initializer) execute(*s.initializer);
    while (true) {
        if (s.condition && !isTruthy(evaluate(*s.condition))) break;
        execute(*s.body);
        if (s.increment) evaluate(*s.increment);
    }
}

// ── 헬퍼 ──────────────────────────────────────────────────────────

void Interpreter::checkNumericPair(const Value& l, const Value& r, int line) const {
    checkNumericOperand(l, line);
    checkNumericOperand(r, line);
}

void Interpreter::checkNumericOperand(const Value& v, int line) const {
    if (!std::holds_alternative<double>(v))
        throw RuntimeError("[라인 " + std::to_string(line)
            + "] 런타임 오류: 피연산자는 반드시 숫자여야 합니다.");
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
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    return std::get<std::string>(v);
}
