#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Expr.h"

struct Stmt {
    virtual ~Stmt() = default;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt {
    ExprPtr m_expr;
    explicit ExprStmt(ExprPtr expr) : m_expr(std::move(expr)) {}
};

struct PrintStmt : Stmt {
    ExprPtr m_expr;
    explicit PrintStmt(ExprPtr expr) : m_expr(std::move(expr)) {}
};

struct VarStmt : Stmt {
    std::string m_name;
    ExprPtr m_initializer;
    VarStmt(std::string name, ExprPtr initializer)
        : m_name(std::move(name)), m_initializer(std::move(initializer)) {}
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> m_stmts;
    explicit BlockStmt(std::vector<StmtPtr> stmts) : m_stmts(std::move(stmts)) {}
};

struct IfStmt : Stmt {
    ExprPtr m_condition;
    StmtPtr m_thenBranch;
    StmtPtr m_elseBranch;
    IfStmt(ExprPtr condition, StmtPtr thenBranch, StmtPtr elseBranch)
        : m_condition(std::move(condition))
        , m_thenBranch(std::move(thenBranch))
        , m_elseBranch(std::move(elseBranch)) {}
};

struct WhileStmt : Stmt {
    ExprPtr m_condition;
    StmtPtr m_body;
    WhileStmt(ExprPtr condition, StmtPtr body)
        : m_condition(std::move(condition)), m_body(std::move(body)) {}
};

struct ForStmt : Stmt {
    StmtPtr m_init;
    ExprPtr m_condition;
    ExprPtr m_increment;
    StmtPtr m_body;
    ForStmt(StmtPtr init, ExprPtr condition, ExprPtr increment, StmtPtr body)
        : m_init(std::move(init))
        , m_condition(std::move(condition))
        , m_increment(std::move(increment))
        , m_body(std::move(body)) {}
};

struct ReturnStmt : Stmt {
    ExprPtr m_value;
    explicit ReturnStmt(ExprPtr value) : m_value(std::move(value)) {}
};
