#pragma once
#include <memory>
#include <vector>
#include "Expr.h"
#include "Token.h"
#include "StmtVisitor.h"

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& v) = 0;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt {
    ExprPtr expression;
    explicit ExprStmt(ExprPtr e) : expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitExprStmt(*this); }
};

struct PrintStmt : Stmt {
    ExprPtr expression;
    explicit PrintStmt(ExprPtr e) : expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitPrintStmt(*this); }
};

struct VarStmt : Stmt {
    Token   name;
    ExprPtr initializer;
    VarStmt(Token n, ExprPtr i)
        : name(std::move(n)), initializer(std::move(i)) {}
    void accept(StmtVisitor& v) override { v.visitVarStmt(*this); }
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
    void accept(StmtVisitor& v) override { v.visitBlockStmt(*this); }
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e)
        : condition(std::move(c))
        , thenBranch(std::move(t))
        , elseBranch(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitIfStmt(*this); }
};

struct ForStmt : Stmt {
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
    ForStmt(StmtPtr i, ExprPtr c, ExprPtr inc, StmtPtr b)
        : initializer(std::move(i))
        , condition(std::move(c))
        , increment(std::move(inc))
        , body(std::move(b)) {}
    void accept(StmtVisitor& v) override { v.visitForStmt(*this); }
};
