#pragma once
#include <memory>
#include "Token.h"
#include "Value.h"

struct Expr { virtual ~Expr() = default; };
using ExprPtr = std::unique_ptr<Expr>;

struct BinaryExpr : Expr {
    ExprPtr left;
    Token   op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {}
};

struct GroupingExpr : Expr {
    ExprPtr expression;
    explicit GroupingExpr(ExprPtr e) : expression(std::move(e)) {}
};

struct LiteralExpr : Expr {
    Value value;
    explicit LiteralExpr(Value v) : value(std::move(v)) {}
};

struct UnaryExpr : Expr {
    Token   op;
    ExprPtr right;
    UnaryExpr(Token op, ExprPtr r) : op(std::move(op)), right(std::move(r)) {}
};

struct VariableExpr : Expr {
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
};

struct AssignExpr : Expr {
    Token   name;
    ExprPtr value;
    AssignExpr(Token n, ExprPtr v)
        : name(std::move(n)), value(std::move(v)) {}
};
