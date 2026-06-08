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
    Token   keyword;            // 표현식의 첫 토큰 (줄 번호용)
    ExprPtr expression;
    ExprStmt(Token kw, ExprPtr e) : keyword(std::move(kw)), expression(std::move(e)) {}
    explicit ExprStmt(ExprPtr e)
        : keyword(Token{TokenType::SEMICOLON, "", std::monostate{}, 0})
        , expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitExprStmt(*this); }
    int  getLine() const override { return keyword.line; }
};

struct PrintStmt : Stmt {
    int     m_line;
    ExprPtr expression;
    PrintStmt(int line, ExprPtr e) : m_line(line), expression(std::move(e)) {}
    explicit PrintStmt(ExprPtr e)  : m_line(0),    expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitPrintStmt(*this); }
    int  getLine() const override { return m_line; }
};

struct VarStmt : Stmt {
    Token   name;
    ExprPtr initializer;
    VarStmt(Token n, ExprPtr i)
        : name(std::move(n)), initializer(std::move(i)) {}
    void accept(StmtVisitor& v) override { v.visitVarStmt(*this); }
    int  getLine() const override { return name.line; }
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(std::vector<StmtPtr> s) : statements(std::move(s)) {}
    void accept(StmtVisitor& v) override { v.visitBlockStmt(*this); }
};

struct IfStmt : Stmt {
    int     m_line;
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(int line, ExprPtr c, StmtPtr t, StmtPtr e)
        : m_line(line)
        , condition(std::move(c))
        , thenBranch(std::move(t))
        , elseBranch(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitIfStmt(*this); }
    int  getLine() const override { return m_line; }
};

struct ForStmt : Stmt {
    int     m_line;
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
    ForStmt(int line, StmtPtr i, ExprPtr c, ExprPtr inc, StmtPtr b)
        : m_line(line)
        , initializer(std::move(i))
        , condition(std::move(c))
        , increment(std::move(inc))
        , body(std::move(b)) {}
    void accept(StmtVisitor& v) override { v.visitForStmt(*this); }
    int  getLine() const override { return m_line; }
};

// ── Chapter 2: 함수 선언 / return ─────────────────────────────────
struct FunctionStmt : Stmt {
    Token                name;
    std::vector<Token>   params;
    std::vector<StmtPtr> body;
    FunctionStmt(Token name, std::vector<Token> params, std::vector<StmtPtr> body)
        : name(std::move(name))
        , params(std::move(params))
        , body(std::move(body)) {}
    void accept(StmtVisitor& v) override { v.visitFunctionStmt(*this); }
    int  getLine() const override { return name.line; }
};

struct ReturnStmt : Stmt {
    Token   keyword;
    ExprPtr value;    // nullptr 이면 return; (nil 반환)
    ReturnStmt(Token keyword, ExprPtr value)
        : keyword(std::move(keyword)), value(std::move(value)) {}
    void accept(StmtVisitor& v) override { v.visitReturnStmt(*this); }
    int  getLine() const override { return keyword.line; }
};
