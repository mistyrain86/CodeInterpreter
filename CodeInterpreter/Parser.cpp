#include "Parser.h"

std::vector<StmtPtr> Parser::parse(std::vector<Token> tokens) {
    m_tokens  = std::move(tokens);
    m_current = 0;
    std::vector<StmtPtr> stmts;
    while (!isAtEnd()) stmts.push_back(parseStatement());
    return stmts;
}

// ── 토큰 헬퍼 ─────────────────────────────────────────────
bool         Parser::isAtEnd() const  { return peek().type == TokenType::END_OF_FILE; }
const Token& Parser::peek() const     { return m_tokens[m_current]; }
const Token& Parser::previous() const { return m_tokens[m_current - 1]; }
const Token& Parser::advance()        { if (!isAtEnd()) m_current++; return previous(); }
bool         Parser::check(TokenType t) const { return !isAtEnd() && peek().type == t; }

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto t : types) { if (check(t)) { advance(); return true; } }
    return false;
}
const Token& Parser::consume(TokenType t, const std::string& msg) {
    if (check(t)) return advance();
    throw error(peek(), msg);
}
ParseError Parser::error(const Token& tok, const std::string& msg) const {
    std::string loc = (tok.type == TokenType::END_OF_FILE)
        ? " (파일 끝)" : " ('" + tok.lexeme + "' 근처)";
    return ParseError("[라인 " + std::to_string(tok.line)
                      + "] 구문 오류: " + msg + loc);
}

// ── 미구현 스텁 (테스트가 추가될 때마다 채워짐) ────────────
StmtPtr  Parser::parseStatement()  { return parseExprStmt(); }
StmtPtr  Parser::parseVarDecl()    { return nullptr; }
StmtPtr  Parser::parsePrintStmt()  { return nullptr; }
StmtPtr  Parser::parseIfStmt()     { return nullptr; }
StmtPtr  Parser::parseForStmt()    { return nullptr; }
StmtPtr  Parser::parseBlock()      { return nullptr; }
StmtPtr Parser::parseExprStmt() {
    auto e = parseExpression();
    consume(TokenType::SEMICOLON, "';'가 필요합니다.");
    return std::make_unique<ExprStmt>(std::move(e));
}
ExprPtr Parser::parseExpression() { return parseAssignment(); }
ExprPtr  Parser::parseAssignment() { return parseEquality(); }
ExprPtr  Parser::parseEquality()   { return parseComparison(); }
ExprPtr  Parser::parseComparison() { return parseTerm(); }
ExprPtr  Parser::parseTerm()       { return parseFactor(); }
ExprPtr  Parser::parseFactor()     { return parseUnary(); }
ExprPtr  Parser::parseUnary()      { return parsePrimary(); }
ExprPtr Parser::parsePrimary() {
    if (match({TokenType::KW_FALSE}))
        return std::make_unique<LiteralExpr>(Value{false});
    if (match({TokenType::KW_TRUE}))
        return std::make_unique<LiteralExpr>(Value{true});
    if (match({TokenType::NUMBER}))
        return std::make_unique<LiteralExpr>(
            Value{std::get<double>(previous().literal)});
    if (match({TokenType::STRING}))
        return std::make_unique<LiteralExpr>(
            Value{std::get<std::string>(previous().literal)});
    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr e = parseExpression();
        consume(TokenType::RIGHT_PAREN, "표현식 뒤에 ')'가 필요합니다.");
        return std::make_unique<GroupingExpr>(std::move(e));
    }
    throw error(peek(), "표현식이 필요합니다.");
}
