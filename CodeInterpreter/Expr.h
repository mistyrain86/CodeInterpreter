#pragma once
#include <memory>
#include "Token.h"
#include "Value.h"
#include "ExprVisitor.h"

struct Expr {
    virtual ~Expr() = default;
    virtual Value accept(ExprVisitor& v) = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct BinaryExpr : Expr {
    ExprPtr left;
    Token   op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {}
    Value accept(ExprVisitor& v) override { return v.visitBinary(*this); }
};

struct GroupingExpr : Expr {
    ExprPtr expression;
    explicit GroupingExpr(ExprPtr e) : expression(std::move(e)) {}
    Value accept(ExprVisitor& v) override { return v.visitGrouping(*this); }
};

struct LiteralExpr : Expr {
    Value value;
    explicit LiteralExpr(Value v) : value(std::move(v)) {}
    Value accept(ExprVisitor& v) override { return v.visitLiteral(*this); }
};

struct UnaryExpr : Expr {
    Token   op;
    ExprPtr right;
    UnaryExpr(Token op, ExprPtr r) : op(std::move(op)), right(std::move(r)) {}
    Value accept(ExprVisitor& v) override { return v.visitUnary(*this); }
};

struct VariableExpr : Expr {
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
    Value accept(ExprVisitor& v) override { return v.visitVariable(*this); }
};

struct AssignExpr : Expr {
    Token   name;
    ExprPtr value;
    AssignExpr(Token n, ExprPtr v)
        : name(std::move(n)), value(std::move(v)) {}
    Value accept(ExprVisitor& v) override { return v.visitAssign(*this); }
};

// ── Chapter 2: 함수 호출 ───────────────────────────────────────────
struct CallExpr : Expr {
    ExprPtr              callee;
    Token                paren;   // 닫는 ')', 에러 리포트용
    std::vector<ExprPtr> args;
    CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> args)
        : callee(std::move(callee))
        , paren(std::move(paren))
        , args(std::move(args)) {}
    Value accept(ExprVisitor& v) override { return v.visitCallExpr(*this); }
};

// ── Chapter 3: 배열 인덱스 ────────────────────────────────────────
struct IndexGetExpr : Expr {
    ExprPtr object;
    Token   bracket;
    ExprPtr index;
    IndexGetExpr(ExprPtr obj, Token bracket, ExprPtr idx)
        : object(std::move(obj))
        , bracket(std::move(bracket))
        , index(std::move(idx)) {}
    Value accept(ExprVisitor& v) override { return v.visitIndexGetExpr(*this); }
};

struct IndexSetExpr : Expr {
    ExprPtr object;
    Token   bracket;
    ExprPtr index;
    ExprPtr value;
    IndexSetExpr(ExprPtr obj, Token bracket, ExprPtr idx, ExprPtr val)
        : object(std::move(obj))
        , bracket(std::move(bracket))
        , index(std::move(idx))
        , value(std::move(val)) {}
    Value accept(ExprVisitor& v) override { return v.visitIndexSetExpr(*this); }
};
