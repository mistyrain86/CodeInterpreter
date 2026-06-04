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
StmtPtr Parser::parseStatement() {
    if (match({TokenType::KW_VAR}))     return parseVarDecl();
    if (match({TokenType::KW_PRINT}))   return parsePrintStmt();
    if (match({TokenType::KW_IF}))      return parseIfStmt();
    if (match({TokenType::KW_FOR}))     return parseForStmt();
    if (match({TokenType::LEFT_BRACE})) return parseBlock();
    return parseExprStmt();
}
StmtPtr Parser::parseVarDecl() {
    Token name = consume(TokenType::IDENTIFIER, "변수 이름이 필요합니다.");
    ExprPtr init;
    if (match({TokenType::EQUAL})) init = parseExpression();
    consume(TokenType::SEMICOLON, "변수 선언 뒤에 ';'가 필요합니다.");
    return std::make_unique<VarStmt>(std::move(name), std::move(init));
}
StmtPtr Parser::parsePrintStmt() {
    ExprPtr val = parseExpression();
    consume(TokenType::SEMICOLON, "값 출력 뒤에 ';'가 필요합니다.");
    return std::make_unique<PrintStmt>(std::move(val));
}
StmtPtr  Parser::parseIfStmt()     { return nullptr; }
StmtPtr  Parser::parseForStmt()    { return nullptr; }
StmtPtr  Parser::parseBlock()      { return nullptr; }
StmtPtr  Parser::parseExprStmt()   { auto e = parseExpression(); consume(TokenType::SEMICOLON, "';'가 필요합니다.");
return std::make_unique<ExprStmt>(std::move(e)); }
ExprPtr  Parser::parseExpression() { return parseAssignment(); }
ExprPtr Parser::parseAssignment() {
    ExprPtr expr = parseEquality();
    if (match({TokenType::EQUAL})) {
        Token eq = previous();
        ExprPtr val = parseAssignment();
        if (auto* v = dynamic_cast<VariableExpr*>(expr.get()))
            return std::make_unique<AssignExpr>(v->name, std::move(val));
        throw error(eq, "잘못된 할당 대상입니다.");
    }
    return expr;
}
ExprPtr Parser::parseEquality() {
    ExprPtr e = parseComparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        e = std::make_unique<BinaryExpr>(std::move(e), op, parseComparison());
    }
    return e;
}
ExprPtr Parser::parseComparison() {
    ExprPtr e = parseTerm();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                  TokenType::LESS,    TokenType::LESS_EQUAL})) {
        Token op = previous();
        e = std::make_unique<BinaryExpr>(std::move(e), op, parseTerm());
    }
    return e;
}
ExprPtr Parser::parseTerm() {
    ExprPtr e = parseFactor();
    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        e = std::make_unique<BinaryExpr>(std::move(e), op, parseFactor());
    }
    return e;
}
ExprPtr Parser::parseFactor() {
    ExprPtr e = parseUnary();
    while (match({TokenType::STAR, TokenType::SLASH})) {
        Token op = previous();
        e = std::make_unique<BinaryExpr>(std::move(e), op, parseUnary());
    }
    return e;
}
ExprPtr Parser::parseUnary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        return std::make_unique<UnaryExpr>(op, parseUnary());
    }
    return parsePrimary();
}
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
    if (match({TokenType::IDENTIFIER}))
        return std::make_unique<VariableExpr>(previous());
    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr e = parseExpression();
        consume(TokenType::RIGHT_PAREN, "표현식 뒤에 ')'가 필요합니다.");
        return std::make_unique<GroupingExpr>(std::move(e));
    }
    throw error(peek(), "표현식이 필요합니다.");
}
