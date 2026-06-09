#include "Interpreter.h"
#include "ArrayBuiltin.h"
#include "ICallable.h"
#include "LangFunction.h"
#include "ReturnSignal.h"
#include <cmath>
#include <iostream>
#include <sstream>

namespace {
static constexpr auto UNIMPLEMENTED_EXPR   = "미구현 표현식 타입";
static constexpr auto UNIMPLEMENTED_UNARY  = "미구현 단항 연산자";
static constexpr auto UNIMPLEMENTED_BINARY = "미구현 이항 연산자";

static std::string runtimeErr(int line, const std::string& msg) {
    return "[라인 " + std::to_string(line) + "] 런타임 오류: " + msg;
}
static bool isExactlyZero(double d) noexcept { return d == 0.0; }

std::pair<std::vector<Value>*, int> resolveArrayAccess(
        const Value& obj, const Value& idx, int line) {
    if (!std::holds_alternative<ArrayType>(obj))
        throw RuntimeError(runtimeErr(line, "인덱스 접근은 배열만 지원합니다."));
    if (!std::holds_alternative<double>(idx))
        throw RuntimeError(runtimeErr(line, "인덱스는 반드시 숫자여야 합니다."));
    auto* arr = std::get<ArrayType>(obj).get();
    int   i   = static_cast<int>(std::get<double>(idx));
    if (i < 0 || i >= static_cast<int>(arr->size()))
        throw RuntimeError(runtimeErr(line, "인덱스 범위를 벗어났습니다. (" + std::to_string(i) + ")"));
    return {arr, i};
}

std::string stringifyDouble(double d) {
    if (std::isfinite(d) && d == std::floor(d))
        return std::to_string(static_cast<long long>(d));
    std::ostringstream oss;
    oss << d;
    return oss.str();
}

std::string stringifyArray(const ArrayType& arr,
                           const std::function<std::string(const Value&)>& recurse) {
    if (!arr) return "[]";
    std::ostringstream oss;
    oss << "[";
    for (std::size_t i = 0; i < arr->size(); ++i) {
        if (i) oss << ", ";
        oss << recurse((*arr)[i]);
    }
    oss << "]";
    return oss.str();
}

struct ScopeGuard {
    std::shared_ptr<Environment>& ref;
    std::shared_ptr<Environment>  prev;
    ScopeGuard(std::shared_ptr<Environment>& r, std::shared_ptr<Environment> next)
        : ref(r), prev(r) { r = std::move(next); }
    ~ScopeGuard() { ref = prev; }
};
}

Interpreter::Interpreter()
    : m_currentEnv(std::make_shared<Environment>()) {
    m_currentEnv->define("Array", Value{std::make_shared<ArrayBuiltin>()});
    m_scopeStack.push_back(m_currentEnv.get());
    initBinaryOps();
}

void Interpreter::initBinaryOps() {
    using T = TokenType;
    m_binaryOps[static_cast<int>(T::PLUS)] = [this](const Value& l, const Value& r, int line) -> Value {
        if (std::holds_alternative<double>(l) && std::holds_alternative<double>(r))
            return std::get<double>(l) + std::get<double>(r);
        if (std::holds_alternative<std::string>(l) && std::holds_alternative<std::string>(r))
            return std::get<std::string>(l) + std::get<std::string>(r);
        throw RuntimeError(runtimeErr(line, "피연산자는 두 숫자 또는 두 문자열이어야 합니다."));
    };
    m_binaryOps[static_cast<int>(T::MINUS)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        return std::get<double>(l) - std::get<double>(r);
    };
    m_binaryOps[static_cast<int>(T::STAR)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        return std::get<double>(l) * std::get<double>(r);
    };
    m_binaryOps[static_cast<int>(T::SLASH)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        const double dr = std::get<double>(r);
        if (isExactlyZero(dr))
            throw RuntimeError(runtimeErr(line, "0으로 나눌 수 없습니다."));
        return std::get<double>(l) / dr;
    };
    m_binaryOps[static_cast<int>(T::PERCENT)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        const double dr = std::get<double>(r);
        if (isExactlyZero(dr))
            throw RuntimeError(runtimeErr(line, "0으로 나눌 수 없습니다."));
        return std::fmod(std::get<double>(l), dr);
    };
    m_binaryOps[static_cast<int>(T::GREATER)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        return std::get<double>(l) > std::get<double>(r);
    };
    m_binaryOps[static_cast<int>(T::GREATER_EQUAL)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        return std::get<double>(l) >= std::get<double>(r);
    };
    m_binaryOps[static_cast<int>(T::LESS)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        return std::get<double>(l) < std::get<double>(r);
    };
    m_binaryOps[static_cast<int>(T::LESS_EQUAL)] = [this](const Value& l, const Value& r, int line) -> Value {
        checkNumericPair(l, r, line);
        return std::get<double>(l) <= std::get<double>(r);
    };
    m_binaryOps[static_cast<int>(T::EQUAL_EQUAL)] = [](const Value& l, const Value& r, int) -> Value {
        return Value{l == r};
    };
    m_binaryOps[static_cast<int>(T::BANG_EQUAL)] = [](const Value& l, const Value& r, int) -> Value {
        return Value{!(l == r)};
    };
}

void Interpreter::rebuildScopeStackFromClosure(Environment* closure) {
    // 클로저 env 체인을 global → closure 순으로 재구성
    std::vector<Environment*> chain;
    auto* cur = closure;
    while (cur) {
        chain.push_back(cur);
        cur = cur->enclosing().get();
    }
    std::reverse(chain.begin(), chain.end());
    m_scopeStack = std::move(chain);
}

void Interpreter::interpret(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) execute(*s);
}

Value Interpreter::evaluate(Expr& expr) {
    return expr.accept(*this);
}

void Interpreter::execute(Stmt& stmt) {
    m_executeDepth++;
    struct Guard { int& d; ~Guard() { --d; } } guard{m_executeDepth};
    if (m_stmtHook) m_stmtHook(stmt);
    stmt.accept(*this);
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& stmts,
                                std::shared_ptr<Environment> env) {
    ScopeGuard guard(m_currentEnv, std::move(env));
    m_scopeStack.push_back(m_currentEnv.get());
    struct StackGuard { std::vector<Environment*>& s; ~StackGuard() { s.pop_back(); } }
        stackGuard{m_scopeStack};
    for (const auto& s : stmts) execute(*s);
}

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

Value Interpreter::visitLogical(LogicalExpr& e) {
    Value left = evaluate(*e.left);
    if (e.op.type == TokenType::KW_OR) {
        if (isTruthy(left)) return Value{true};
        return Value{isTruthy(evaluate(*e.right))};
    }
    if (e.op.type == TokenType::KW_AND) {
        if (!isTruthy(left)) return Value{false};
        return Value{isTruthy(evaluate(*e.right))};
    }
    throw RuntimeError(UNIMPLEMENTED_BINARY);
}

Value Interpreter::visitBinary(BinaryExpr& e) {
    if (m_opSpy) m_opSpy->m_binaryOpCount++;
    Value l = evaluate(*e.left);
    Value r = evaluate(*e.right);
    auto it = m_binaryOps.find(static_cast<int>(e.op.type));
    if (it != m_binaryOps.end()) return it->second(l, r, e.op.line);
    throw RuntimeError(UNIMPLEMENTED_BINARY);
}

std::vector<std::string> Interpreter::globalNames() const {
    std::vector<std::string> result;
    const auto& vals = m_currentEnv->values();
    result.reserve(vals.size());
    for (const auto& [name, val] : vals)
        result.push_back(name);
    return result;
}

std::optional<int> Interpreter::lookupBinding(const Expr* expr) const {
    if (!m_bindings) return std::nullopt;
    auto it = m_bindings->find(expr);
    if (it == m_bindings->end()) return std::nullopt;
    return it->second;
}

Value Interpreter::visitVariable(VariableExpr& e) {
    if (auto dist = lookupBinding(&e)) {
        if (m_spy) m_spy->m_bindingHits++;
        // O(1): 평탄화 스코프 스택 배열 인덱스로 직접 접근 (체인 순회 없음)
        int idx = static_cast<int>(m_scopeStack.size()) - 1 - *dist;
        return m_scopeStack[idx]->values().at(e.name.lexeme);
    }
    if (m_spy) m_spy->m_chainWalks++;
    return m_currentEnv->get(e.name);
}

Value Interpreter::visitAssign(AssignExpr& e) {
    Value v = evaluate(*e.value);
    if (auto dist = lookupBinding(&e)) {
        if (m_spy) m_spy->m_bindingHits++;
        // O(1): 평탄화 스코프 스택 배열 인덱스로 직접 접근 (체인 순회 없음)
        int idx = static_cast<int>(m_scopeStack.size()) - 1 - *dist;
        m_scopeStack[idx]->values()[e.name.lexeme] = v;
        return v;
    }
    if (m_spy) m_spy->m_chainWalks++;
    m_currentEnv->assign(e.name, v);
    return v;
}

void Interpreter::visitExprStmt(ExprStmt& s) {
    evaluate(*s.m_expression);
}

void Interpreter::visitPrintStmt(PrintStmt& s) {
    std::cout << stringify(evaluate(*s.m_expression)) << "\n";
}

void Interpreter::visitVarStmt(VarStmt& s) {
    Value v = s.m_initializer ? evaluate(*s.m_initializer) : Value{std::monostate{}};
    m_currentEnv->define(s.m_name.lexeme, std::move(v));
}

void Interpreter::visitBlockStmt(BlockStmt& s) {
    executeBlock(s.m_statements, std::make_shared<Environment>(m_currentEnv));
}

void Interpreter::visitIfStmt(IfStmt& s) {
    if (isTruthy(evaluate(*s.m_condition))) execute(*s.m_thenBranch);
    else if (s.m_elseBranch)               execute(*s.m_elseBranch);
}

void Interpreter::visitForStmt(ForStmt& s) {
    if (!s.m_body)
        throw RuntimeError("런타임 오류: ForStmt body가 null입니다.");
    ScopeGuard guard(m_currentEnv, std::make_shared<Environment>(m_currentEnv));
    m_scopeStack.push_back(m_currentEnv.get());
    struct StackGuard { std::vector<Environment*>& s; ~StackGuard() { s.pop_back(); } }
        stackGuard{m_scopeStack};
    if (s.m_initializer) execute(*s.m_initializer);
    while (true) {
        if (s.m_condition && !isTruthy(evaluate(*s.m_condition))) break;
        execute(*s.m_body);
        if (s.m_increment) evaluate(*s.m_increment);
    }
}

void Interpreter::checkNumericPair(const Value& l, const Value& r, int line) const {
    checkNumericOperand(l, line);
    checkNumericOperand(r, line);
}

void Interpreter::checkNumericOperand(const Value& v, int line) const {
    if (!std::holds_alternative<double>(v))
        throw RuntimeError(runtimeErr(line, "피연산자는 반드시 숫자여야 합니다."));
}

bool Interpreter::isTruthy(const Value& v) const {
    if (std::holds_alternative<std::monostate>(v)) return false;
    if (std::holds_alternative<bool>(v))           return std::get<bool>(v);
    if (std::holds_alternative<double>(v))         return std::get<double>(v) != 0.0;
    return true;
}

void Interpreter::visitFunctionStmt(FunctionStmt& s) {
    auto fn = std::make_shared<LangFunction>(s, m_currentEnv);
    m_currentEnv->define(s.m_name.lexeme, Value{fn});
}

Value Interpreter::visitCallExpr(CallExpr& e) {
    Value callee = evaluate(*e.callee);

    if (!std::holds_alternative<std::shared_ptr<ICallable>>(callee))
        throw RuntimeError(runtimeErr(e.paren.line, "함수가 아닌 대상을 호출했습니다."));

    auto fn = std::get<std::shared_ptr<ICallable>>(callee);

    if (static_cast<int>(e.args.size()) != fn->arity())
        throw RuntimeError(runtimeErr(e.paren.line, "인자 개수 불일치. 기대: "
            + std::to_string(fn->arity())
            + ", 실제: " + std::to_string(e.args.size())));

    std::vector<Value> args;
    for (auto& arg : e.args) args.push_back(evaluate(*arg));

    return fn->call(*this, args);
}

void Interpreter::visitReturnStmt(ReturnStmt& s) {
    Value val = s.m_value ? evaluate(*s.m_value) : Value{std::monostate{}};
    throw ReturnSignal(std::move(val));
}

Value Interpreter::visitIndexGetExpr(IndexGetExpr& e) {
    Value obj = evaluate(*e.object);
    Value idx = evaluate(*e.index);
    auto [arr, i] = resolveArrayAccess(obj, idx, e.bracket.line);
    return (*arr)[i];
}

Value Interpreter::visitIndexSetExpr(IndexSetExpr& e) {
    Value obj = evaluate(*e.object);
    Value idx = evaluate(*e.index);
    Value val = evaluate(*e.value);
    auto [arr, i] = resolveArrayAccess(obj, idx, e.bracket.line);
    (*arr)[i] = val;
    return val;
}

std::string Interpreter::stringify(const Value& v) const {
    if (std::holds_alternative<std::monostate>(v))              return "null";
    if (std::holds_alternative<bool>(v))                        return std::get<bool>(v) ? "true" : "false";
    if (std::holds_alternative<double>(v))                      return stringifyDouble(std::get<double>(v));
    if (std::holds_alternative<std::shared_ptr<ICallable>>(v)) {
        auto& fn = std::get<std::shared_ptr<ICallable>>(v);
        return fn ? "<fn " + fn->name() + ">" : "<fn>";
    }
    if (std::holds_alternative<ArrayType>(v))
        return stringifyArray(std::get<ArrayType>(v),
                              [this](const Value& e) { return stringify(e); });
    return std::get<std::string>(v);
}
