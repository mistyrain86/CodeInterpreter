#pragma once
#include <memory>
#include <vector>
#include "Expr.h"
#include "Token.h"
#include "StmtVisitor.h"

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& v) = 0;
    virtual int  getLine() const { return 0; }  // 디버거용 줄 번호
};
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt : Stmt {
    Token   m_keyword;          // 표현식의 첫 토큰 (줄 번호용)
    ExprPtr m_expression;
    ExprStmt(Token kw, ExprPtr e) : m_keyword(std::move(kw)), m_expression(std::move(e)) {}
    explicit ExprStmt(ExprPtr e)
        : m_keyword(Token{TokenType::SEMICOLON, "", std::monostate{}, 0})
        , m_expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitExprStmt(*this); }
    int  getLine() const override { return m_keyword.line; }
};

struct PrintStmt : Stmt {
    int     m_line;
    ExprPtr m_expression;
    PrintStmt(int line, ExprPtr e) : m_line(line), m_expression(std::move(e)) {}
    explicit PrintStmt(ExprPtr e)  : m_line(0),    m_expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitPrintStmt(*this); }
    int  getLine() const override { return m_line; }
};

struct VarStmt : Stmt {
    Token   m_name;
    ExprPtr m_initializer;
    VarStmt(Token n, ExprPtr i)
        : m_name(std::move(n)), m_initializer(std::move(i)) {}
    void accept(StmtVisitor& v) override { v.visitVarStmt(*this); }
    int  getLine() const override { return m_name.line; }
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> m_statements;
    explicit BlockStmt(std::vector<StmtPtr> s) : m_statements(std::move(s)) {}
    void accept(StmtVisitor& v) override { v.visitBlockStmt(*this); }
};

struct IfStmt : Stmt {
    int     m_line;
    ExprPtr m_condition;
    StmtPtr m_thenBranch;
    StmtPtr m_elseBranch;
    IfStmt(int line, ExprPtr c, StmtPtr t, StmtPtr e)
        : m_line(line)
        , m_condition(std::move(c))
        , m_thenBranch(std::move(t))
        , m_elseBranch(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitIfStmt(*this); }
    int  getLine() const override { return m_line; }
};

struct ForStmt : Stmt {
    int     m_line;
    StmtPtr m_initializer;
    ExprPtr m_condition;
    ExprPtr m_increment;
    StmtPtr m_body;
    ForStmt(int line, StmtPtr i, ExprPtr c, ExprPtr inc, StmtPtr b)
        : m_line(line)
        , m_initializer(std::move(i))
        , m_condition(std::move(c))
        , m_increment(std::move(inc))
        , m_body(std::move(b)) {}
    void accept(StmtVisitor& v) override { v.visitForStmt(*this); }
    int  getLine() const override { return m_line; }
};

// ── Chapter 2: 함수 선언 / return ─────────────────────────────────
struct FunctionStmt : Stmt {
    Token                m_name;
    std::vector<Token>   m_params;
    std::vector<StmtPtr> m_body;
    FunctionStmt(Token name, std::vector<Token> params, std::vector<StmtPtr> body)
        : m_name(std::move(name))
        , m_params(std::move(params))
        , m_body(std::move(body)) {}
    void accept(StmtVisitor& v) override { v.visitFunctionStmt(*this); }
    int  getLine() const override { return m_name.line; }
};

struct ReturnStmt : Stmt {
    Token   m_keyword;
    ExprPtr m_value;    // nullptr 이면 return; (null 반환)
    ReturnStmt(Token keyword, ExprPtr value)
        : m_keyword(std::move(keyword)), m_value(std::move(value)) {}
    void accept(StmtVisitor& v) override { v.visitReturnStmt(*this); }
    int  getLine() const override { return m_keyword.line; }
};
