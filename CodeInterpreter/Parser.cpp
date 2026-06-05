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

StmtPtr Parser::parseStatement() {
    // Ch.2: 함수 선언 / return
    if (match({TokenType::KW_FUNC}))    return parseFunctionStmt();
    if (match({TokenType::KW_RETURN}))  return parseReturnStmt();
    // Ch.1: 기존 문장
    if (match({TokenType::KW_VAR}))     return parseVarDecl();
    if (match({TokenType::KW_PRINT}))   return parsePrintStmt();
    if (match({TokenType::KW_IF}))      return parseIfStmt();
    if (match({TokenType::KW_FOR}))     return parseForStmt();
    if (match({TokenType::LEFT_BRACE})) return parseBlock();
    return parseExprStmt();
}
StmtPtr Parser::parseFunctionStmt() {
    Token name = consume(TokenType::IDENTIFIER, "함수 이름이 필요합니다.");
    consume(TokenType::LEFT_PAREN, "함수 이름 뒤에 '('가 필요합니다.");
    std::vector<Token> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            params.push_back(
                consume(TokenType::IDENTIFIER, "파라미터 이름이 필요합니다."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "파라미터 목록 뒤에 ')'가 필요합니다.");
    consume(TokenType::LEFT_BRACE,  "함수 본문 앞에 '{'가 필요합니다.");
    std::vector<StmtPtr> body;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        body.push_back(parseStatement());
    consume(TokenType::RIGHT_BRACE, "함수 본문 뒤에 '}'가 필요합니다.");
    return std::make_unique<FunctionStmt>(
        std::move(name), std::move(params), std::move(body));
}
StmtPtr Parser::parseReturnStmt() {
    Token   keyword = previous();
    ExprPtr value;  // unique_ptr 기본값 = nullptr
    if (!check(TokenType::SEMICOLON))
        value = parseExpression();
    consume(TokenType::SEMICOLON, "return 뒤에 ';'가 필요합니다.");
    return std::make_unique<ReturnStmt>(std::move(keyword), std::move(value));
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
StmtPtr Parser::parseIfStmt() {
    consume(TokenType::LEFT_PAREN,  "if 뒤에 '('가 필요합니다.");
    ExprPtr cond = parseExpression();
    consume(TokenType::RIGHT_PAREN, "조건식 뒤에 ')'가 필요합니다.");
    StmtPtr thenB = parseStatement();
    StmtPtr elseB;
    if (match({TokenType::KW_ELSE})) elseB = parseStatement(); // Greedy 매칭
    return std::make_unique<IfStmt>(std::move(cond), std::move(thenB), std::move(elseB));
}
StmtPtr Parser::parseForStmt() {
    consume(TokenType::LEFT_PAREN, "for 뒤에 '('가 필요합니다.");
    StmtPtr init;
    if      (match({TokenType::SEMICOLON})) { /* empty */ }
    else if (match({TokenType::KW_VAR}))    init = parseVarDecl();
    else                                    init = parseExprStmt();
    ExprPtr cond;
    if (!check(TokenType::SEMICOLON)) cond = parseExpression();
    consume(TokenType::SEMICOLON, "for 조건식 뒤에 ';'가 필요합니다.");
    ExprPtr incr;
    if (!check(TokenType::RIGHT_PAREN)) incr = parseExpression();
    consume(TokenType::RIGHT_PAREN, "for 증감식 뒤에 ')'가 필요합니다.");
    return std::make_unique<ForStmt>(std::move(init), std::move(cond),
                                     std::move(incr), parseStatement());
}
StmtPtr Parser::parseBlock() {
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
        stmts.push_back(parseStatement());
    consume(TokenType::RIGHT_BRACE, "블록 뒤에 '}'가 필요합니다.");
    return std::make_unique<BlockStmt>(std::move(stmts));
}
StmtPtr Parser::parseExprStmt() {
    auto e = parseExpression();
    consume(TokenType::SEMICOLON, "';'가 필요합니다.");
    return std::make_unique<ExprStmt>(std::move(e));
}
ExprPtr Parser::parseExpression() { return parseAssignment(); }
ExprPtr Parser::parseAssignment() {
    ExprPtr expr = parseEquality();
    if (match({TokenType::EQUAL})) {
        Token   eq    = previous();  // '=' 토큰 즉시 캡처 — 에러 위치 보고용
        ExprPtr value = parseAssignment();
        // 변수 대입: a = v
        if (auto* v = dynamic_cast<VariableExpr*>(expr.get()))
            return std::make_unique<AssignExpr>(v->name, std::move(value));
        // 배열 원소 대입: arr[i] = v
        if (auto* idx = dynamic_cast<IndexGetExpr*>(expr.get()))
            return std::make_unique<IndexSetExpr>(
                std::move(idx->object),
                idx->bracket,
                std::move(idx->index),
                std::move(value));
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
    return parseCall();
}
ExprPtr Parser::parseCall() {
    ExprPtr expr = parsePrimary();
    // 후위 연산자 체인: f(args)[idx] 형태를 좌결합으로 처리
    while (true) {
        if (match({TokenType::LEFT_PAREN})) {          // 함수 호출
            expr = finishCall(std::move(expr));
        } else if (match({TokenType::LEFT_BRACKET})) { // 배열 인덱스
            Token   bracket = previous();
            ExprPtr index   = parseExpression();
            consume(TokenType::RIGHT_BRACKET, "인덱스 뒤에 ']'가 필요합니다.");
            expr = std::make_unique<IndexGetExpr>(
                std::move(expr), std::move(bracket), std::move(index));
        } else {
            break;
        }
    }
    return expr;
}
ExprPtr Parser::finishCall(ExprPtr callee) {
    std::vector<ExprPtr> args;
    if (!check(TokenType::RIGHT_PAREN)) {
        do { args.push_back(parseExpression()); }
        while (match({TokenType::COMMA}));
    }
    Token paren = consume(TokenType::RIGHT_PAREN, "인자 목록 뒤에 ')'가 필요합니다.");
    return std::make_unique<CallExpr>(
        std::move(callee), std::move(paren), std::move(args));
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
