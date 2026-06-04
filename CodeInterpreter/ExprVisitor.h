#pragma once
#include "Value.h"

// 전방 선언
struct LiteralExpr;
struct GroupingExpr;
struct UnaryExpr;
struct BinaryExpr;
struct VariableExpr;
struct AssignExpr;

struct ExprVisitor {
    virtual ~ExprVisitor() = default;
    virtual Value visitLiteral (LiteralExpr&)  = 0;
    virtual Value visitGrouping(GroupingExpr&) = 0;
    virtual Value visitUnary   (UnaryExpr&)    = 0;
    virtual Value visitBinary  (BinaryExpr&)   = 0;
    virtual Value visitVariable(VariableExpr&) = 0;
    virtual Value visitAssign  (AssignExpr&)   = 0;
};
