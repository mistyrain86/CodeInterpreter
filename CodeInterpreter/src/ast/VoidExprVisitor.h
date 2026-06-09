#pragma once

struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;
struct LogicalExpr;
struct VariableExpr;
struct AssignExpr;
struct CallExpr;
struct IndexGetExpr;
struct IndexSetExpr;

struct VoidExprVisitor {
    virtual ~VoidExprVisitor() = default;
    virtual void visitLiteral     (LiteralExpr&)   {}
    virtual void visitGrouping    (GroupingExpr&)  = 0;
    virtual void visitUnary       (UnaryExpr&)     = 0;
    virtual void visitBinary      (BinaryExpr&)    = 0;
    virtual void visitLogical     (LogicalExpr&)   = 0;
    virtual void visitVariable    (VariableExpr&)  = 0;
    virtual void visitAssign      (AssignExpr&)    = 0;
    virtual void visitCallExpr    (CallExpr&)      = 0;
    virtual void visitIndexGetExpr(IndexGetExpr&)  = 0;
    virtual void visitIndexSetExpr(IndexSetExpr&)  = 0;
};
