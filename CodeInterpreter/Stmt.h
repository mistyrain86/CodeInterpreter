#pragma once
#include <memory>
#include <vector>
#include "Expr.h"
#include "Token.h"

struct Stmt { virtual ~Stmt() = default; };
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt {
    ExprPtr expression;
    explicit ExprStmt(ExprPtr e) : expression(std::move(e)) {}
};

struct PrintStmt : Stmt {
    ExprPtr expression;
    explicit PrintStmt(ExprPtr e) : expression(std::move(e)) {}
};

struct VarStmt : Stmt {
    Token   name;
    ExprPtr initializer;
    VarStmt(Token n, ExprPtr i)
        : name(std::move(n)), initializer(std::move(i)) {}
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e)
        : condition(std::move(c))
        , thenBranch(std::move(t))
        , elseBranch(std::move(e)) {}
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
};
