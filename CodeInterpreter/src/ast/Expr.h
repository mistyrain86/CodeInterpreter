#pragma once
#include <memory>
#include "Token.h"
#include "Value.h"
#include "ExprVisitor.h"
#include "VoidExprVisitor.h"

struct Expr {
    virtual ~Expr() = default;
    virtual Value accept    (ExprVisitor&     v) = 0;
    virtual void  acceptVoid(VoidExprVisitor& v) = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct BinaryExpr : Expr {
    ExprPtr left;
    Token   op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitBinary(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitBinary(*this); }
};

struct LogicalExpr : Expr {
    ExprPtr left;
    Token   op;
    ExprPtr right;
    LogicalExpr(ExprPtr l, Token op, ExprPtr r)
        : left(std::move(l)), op(std::move(op)), right(std::move(r)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitLogical(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitLogical(*this); }
};

struct GroupingExpr : Expr {
    ExprPtr expression;
    explicit GroupingExpr(ExprPtr e) : expression(std::move(e)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitGrouping(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitGrouping(*this); }
};

struct LiteralExpr : Expr {
    Value value;
    explicit LiteralExpr(Value v) : value(std::move(v)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitLiteral(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitLiteral(*this); }
};

struct UnaryExpr : Expr {
    Token   op;
    ExprPtr right;
    UnaryExpr(Token op, ExprPtr r) : op(std::move(op)), right(std::move(r)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitUnary(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitUnary(*this); }
};

struct VariableExpr : Expr {
    Token name;
    explicit VariableExpr(Token n) : name(std::move(n)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitVariable(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitVariable(*this); }
};

struct AssignExpr : Expr {
    Token   name;
    ExprPtr value;
    AssignExpr(Token n, ExprPtr v)
        : name(std::move(n)), value(std::move(v)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitAssign(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitAssign(*this); }
};

struct CallExpr : Expr {
    ExprPtr              callee;
    Token                paren;
    std::vector<ExprPtr> args;
    CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> args)
        : callee(std::move(callee))
        , paren(std::move(paren))
        , args(std::move(args)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitCallExpr(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitCallExpr(*this); }
};

struct IndexGetExpr : Expr {
    ExprPtr object;
    Token   bracket;
    ExprPtr index;
    IndexGetExpr(ExprPtr obj, Token bracket, ExprPtr idx)
        : object(std::move(obj))
        , bracket(std::move(bracket))
        , index(std::move(idx)) {}
    Value accept    (ExprVisitor& v)     override { return v.visitIndexGetExpr(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitIndexGetExpr(*this); }
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
    Value accept    (ExprVisitor& v)     override { return v.visitIndexSetExpr(*this); }
    void  acceptVoid(VoidExprVisitor& v) override { v.visitIndexSetExpr(*this); }
};
