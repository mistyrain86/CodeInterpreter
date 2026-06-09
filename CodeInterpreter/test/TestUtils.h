#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include "Expr.h"
#include "LangFactory.h"
#include "Stmt.h"
#include "Token.h"

inline std::string captureOutput(std::function<void()> fn) {
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    fn();
    std::cout.rdbuf(old);
    return oss.str();
}

inline Token makeIdent(std::string name, int line = 1) {
    return Token{TokenType::IDENTIFIER, std::move(name), std::monostate{}, line};
}
inline ExprPtr litNum(double v) {
    return std::make_unique<LiteralExpr>(Value{v});
}
inline ExprPtr litStr(std::string s) {
    return std::make_unique<LiteralExpr>(Value{std::move(s)});
}
inline ExprPtr litBool(bool b) {
    return std::make_unique<LiteralExpr>(Value{b});
}
inline ExprPtr varRef(std::string name, int line = 1) {
    return std::make_unique<VariableExpr>(makeIdent(std::move(name), line));
}
inline StmtPtr varDecl(std::string name, ExprPtr init, int line = 1) {
    return std::make_unique<VarStmt>(
        makeIdent(std::move(name), line), std::move(init));
}
inline StmtPtr printStmt(ExprPtr expr) {
    return std::make_unique<PrintStmt>(std::move(expr));
}
inline StmtPtr blockStmt(std::vector<StmtPtr> stmts) {
    return std::make_unique<BlockStmt>(std::move(stmts));
}
inline ExprPtr binaryExpr(ExprPtr l, TokenType op, std::string lex, ExprPtr r) {
    return std::make_unique<BinaryExpr>(
        std::move(l), Token{op, std::move(lex), std::monostate{}, 1}, std::move(r));
}
inline ExprPtr logicalExpr(ExprPtr l, TokenType op, std::string lex, ExprPtr r) {
    return std::make_unique<LogicalExpr>(
        std::move(l), Token{op, std::move(lex), std::monostate{}, 1}, std::move(r));
}
inline std::string execSource(const std::string& src) {
    return captureOutput([&]{
        LangFactory factory;
        factory.run(src);
    });
}
