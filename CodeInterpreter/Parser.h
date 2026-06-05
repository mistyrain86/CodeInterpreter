#pragma once
#include <functional>
#include <initializer_list>
#include <vector>
#include "IParser.h"
#include "ParseError.h"
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"

class Parser : public IParser {
public:
    Parser() = default;
    std::vector<StmtPtr> parse(std::vector<Token> tokens) override;

private:
    std::vector<Token> m_tokens;
    int                m_current = 0;

    // ── 문장 파서 ──────────────────────────────────────────────
    StmtPtr  parseStatement();
    StmtPtr  parseVarDecl();
    StmtPtr  parsePrintStmt();
    StmtPtr  parseIfStmt();
    StmtPtr  parseForStmt();
    StmtPtr  parseBlock();
    StmtPtr  parseExprStmt();
    StmtPtr  parseFunctionStmt();  // Ch.2
    StmtPtr  parseReturnStmt();    // Ch.2

    // ── 표현식 파서 ────────────────────────────────────────────
    ExprPtr  parseExpression();
    ExprPtr  parseAssignment();
    ExprPtr  parseEquality();
    ExprPtr  parseComparison();
    ExprPtr  parseTerm();
    ExprPtr  parseFactor();
    ExprPtr  parseUnary();
    ExprPtr  parseCall();           // Ch.2/3
    ExprPtr  finishCall(ExprPtr callee);
    ExprPtr  parsePrimary();
    ExprPtr  parseBinaryLeft(std::initializer_list<TokenType> ops,
                              std::function<ExprPtr()>         next);

    // ── 토큰 유틸리티 ──────────────────────────────────────────
    bool         isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool         check(TokenType t) const;
    bool         match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType t, const std::string& msg);
    ParseError   error(const Token& tok, const std::string& msg) const;
};
