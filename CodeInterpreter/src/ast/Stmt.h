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
    int     line = 0;
    ExprPtr expression;
    explicit PrintStmt(ExprPtr e, int ln = 0) : expression(std::move(e)), line(ln) {}
    void accept(StmtVisitor& v) override { v.visitPrintStmt(*this); }
    int  getLine() const override { return line; }
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
    int     line = 0;
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(ExprPtr c, StmtPtr t, StmtPtr e, int ln = 0)
        : condition(std::move(c))
        , thenBranch(std::move(t))
        , elseBranch(std::move(e))
        , line(ln) {}
    void accept(StmtVisitor& v) override { v.visitIfStmt(*this); }
    int  getLine() const override { return line; }
};

struct ForStmt : Stmt {
    int     line = 0;
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
    ForStmt(StmtPtr i, ExprPtr c, ExprPtr inc, StmtPtr b, int ln = 0)
        : initializer(std::move(i))
        , condition(std::move(c))
        , increment(std::move(inc))
        , body(std::move(b))
        , line(ln) {}
    void accept(StmtVisitor& v) override { v.visitForStmt(*this); }
    int  getLine() const override { return line; }
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
