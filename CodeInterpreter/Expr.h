#pragma once
#include <memory>
#include <string>
#include "Value.h"

struct Expr {
    virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

struct LiteralExpr : Expr {
    Value m_value;
    explicit LiteralExpr(Value value) : m_value(std::move(value)) {}
};

struct VariableExpr : Expr {
    std::string m_name;
    explicit VariableExpr(std::string name) : m_name(std::move(name)) {}
};

struct AssignExpr : Expr {
    std::string m_name;
    ExprPtr m_value;
    AssignExpr(std::string name, ExprPtr value)
        : m_name(std::move(name)), m_value(std::move(value)) {}
};

struct BinaryExpr : Expr {
    ExprPtr m_left;
    std::string m_op;
    ExprPtr m_right;
    BinaryExpr(ExprPtr left, std::string op, ExprPtr right)
        : m_left(std::move(left)), m_op(std::move(op)), m_right(std::move(right)) {}
};

struct UnaryExpr : Expr {
    std::string m_op;
    ExprPtr m_right;
    UnaryExpr(std::string op, ExprPtr right)
        : m_op(std::move(op)), m_right(std::move(right)) {}
};

struct GroupingExpr : Expr {
    ExprPtr m_expr;
    explicit GroupingExpr(ExprPtr expr) : m_expr(std::move(expr)) {}
};

struct LogicalExpr : Expr {
    ExprPtr m_left;
    std::string m_op;
    ExprPtr m_right;
    LogicalExpr(ExprPtr left, std::string op, ExprPtr right)
        : m_left(std::move(left)), m_op(std::move(op)), m_right(std::move(right)) {}
};
