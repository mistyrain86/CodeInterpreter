#pragma once
#include <initializer_list>
#include <stdexcept>
#include <vector>
#include "IParser.h"
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class Parser : public IParser {
public:
    Parser() = default;
    std::vector<StmtPtr> parse(std::vector<Token> tokens) override;

private:
    std::vector<Token> m_tokens;
    int                m_current = 0;

    StmtPtr  parseStatement();
    StmtPtr  parseVarDecl();
    StmtPtr  parsePrintStmt();
    StmtPtr  parseIfStmt();
    StmtPtr  parseForStmt();
    StmtPtr  parseBlock();
    StmtPtr  parseExprStmt();

    ExprPtr  parseExpression();
    ExprPtr  parseAssignment();
    ExprPtr  parseEquality();
    ExprPtr  parseComparison();
    ExprPtr  parseTerm();
    ExprPtr  parseFactor();
    ExprPtr  parseUnary();
    ExprPtr  parsePrimary();

    bool         isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool         check(TokenType t) const;
    bool         match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType t, const std::string& msg);
    ParseError   error(const Token& tok, const std::string& msg) const;
};
