#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include "IParser.h"
#include "TokenStream.h"
#include "Expr.h"
#include "Stmt.h"

class Parser : public IParser {
public:
    Parser();
    std::vector<StmtPtr> parse(std::vector<Token> tokens) override;

private:
    TokenStream m_stream;

    using StmtParserFn = std::function<StmtPtr()>;
    std::unordered_map<int, StmtParserFn> m_stmtDispatch;
    void initDispatch();

    StmtPtr  parseStatement();
    StmtPtr  parseVarDecl();
    StmtPtr  parsePrintStmt();
    StmtPtr  parseIfStmt();
    StmtPtr  parseForStmt();
    StmtPtr  parseBlock();
    StmtPtr  parseExprStmt();
    StmtPtr  parseFunctionStmt();
    StmtPtr  parseReturnStmt();

    ExprPtr  parseExpression();
    ExprPtr  parseAssignment();
    ExprPtr  parseEquality();
    ExprPtr  parseComparison();
    ExprPtr  parseTerm();
    ExprPtr  parseFactor();
    ExprPtr  parseUnary();
    ExprPtr  parseCall();
    ExprPtr  finishCall(ExprPtr callee);
    ExprPtr  parsePrimary();
    ExprPtr  parseBinaryLeft(std::initializer_list<TokenType> ops,
                              std::function<ExprPtr()>         next);

    bool         isAtEnd()                const { return m_stream.isAtEnd(); }
    const Token& peek()                   const { return m_stream.peek(); }
    const Token& previous()               const { return m_stream.previous(); }
    const Token& advance()                      { return m_stream.advance(); }
    bool         check(TokenType t)       const { return m_stream.check(t); }
    bool         match(std::initializer_list<TokenType> types) { return m_stream.match(types); }
    const Token& consume(TokenType t, const std::string& msg)  { return m_stream.consume(t, msg); }
    ParseError   error(const Token& tok,  const std::string& msg) const { return m_stream.error(tok, msg); }
};
