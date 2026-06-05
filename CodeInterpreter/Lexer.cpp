#include "Lexer.h"
#include <stdexcept>
#include <cctype>

const std::unordered_map<std::string, TokenType> Lexer::s_keywords = {
    {"var",    TokenType::KW_VAR},
    {"print",  TokenType::KW_PRINT},
    {"if",     TokenType::KW_IF},
    {"else",   TokenType::KW_ELSE},
    {"for",    TokenType::KW_FOR},
    {"true",   TokenType::KW_TRUE},
    {"false",  TokenType::KW_FALSE},
    {"func",   TokenType::KW_FUNC},
    {"return", TokenType::KW_RETURN},
};

std::vector<Token> Lexer::tokenize(const std::string& source) {
    reset(source);

    while (!isAtEnd()) {
        m_startIdx = m_currentIdx;
        scanToken();
    }

    m_tokens.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, m_line);
    return m_tokens;
}

void Lexer::reset(const std::string& source) {
    m_source = source;
    m_tokens.clear();
    m_startIdx = m_currentIdx = 0;
    m_line = 1;
}

bool Lexer::isAtEnd() const {
    return m_currentIdx >= m_source.size();
}

void Lexer::scanToken() {
    char singleChar = advance();

    if (isWhitespace(singleChar)) return;
    if (singleChar == '\n') { m_line++; return; }

    if (scanPunctuatorAndOperator(singleChar)) return;

    if (singleChar == '"') { scanString(); return; }
    if (isDigit(singleChar)) { scanNumber(); return; }
    if (isAlphaOrUnderscore(singleChar)) { scanIdentifier(); return; }

    runtimeErrorUnexpectedChar(singleChar);
}

bool Lexer::isWhitespace(char c) const {
    return c == ' ' || c == '\r' || c == '\t';
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::isAlphaOrUnderscore(char c) const {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Lexer::scanPunctuatorAndOperator(char singleChar) {
    switch (singleChar) {
    case '(': addToken(TokenType::LEFT_PAREN);    return true;
    case ')': addToken(TokenType::RIGHT_PAREN);   return true;
    case '{': addToken(TokenType::LEFT_BRACE);    return true;
    case '}': addToken(TokenType::RIGHT_BRACE);   return true;
    case '[': addToken(TokenType::LEFT_BRACKET);  return true;
    case ']': addToken(TokenType::RIGHT_BRACKET); return true;
    case ';': addToken(TokenType::SEMICOLON);     return true;
    case ',': addToken(TokenType::COMMA);         return true;
    case '+': addToken(TokenType::PLUS);          return true;
    case '-': addToken(TokenType::MINUS);         return true;
    case '*': addToken(TokenType::STAR);          return true;

    case '/':
        if (match('/')) skipLineComment();
        else            addToken(TokenType::SLASH);
        return true;

    case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);    return true;
    case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);   return true;
    case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);    return true;
    case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); return true;

    default: return false;
    }
}

void Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') m_line++;
        advance();
    }

    if (isAtEnd()) {
        throw std::runtime_error("[라인 " + std::to_string(m_line) + "] 어휘 오류: 문자열이 닫히지 않았습니다.");
    }

    advance(); // 닫는 따옴표('\"') 소비

    constexpr size_t quoteLength = 1;
    size_t stringLength = m_currentIdx - m_startIdx - (quoteLength * 2);
    std::string val = m_source.substr(m_startIdx + quoteLength, stringLength);

    addToken(TokenType::STRING, std::move(val));
}

void Lexer::scanNumber() {
    advanceDigits();

    if (peek() == '.' && peekNext()) {
        advance(); // '.' 소비
        advanceDigits();
    }

    double val = std::stod(m_source.substr(m_startIdx, m_currentIdx - m_startIdx));
    addToken(TokenType::NUMBER, val);
}

void Lexer::scanIdentifier() {
    while (isAlphaOrUnderscore(peek()) || isDigit(peek())) {
        advance();
    }

    std::string text = m_source.substr(m_startIdx, m_currentIdx - m_startIdx);
    auto it = s_keywords.find(text);

    addToken(it != s_keywords.end() ? it->second : TokenType::IDENTIFIER);
}

char Lexer::advance() {
    return m_source[m_currentIdx++];
}

void Lexer::advanceDigits() {
    while (isDigit(peek())) {
        advance();
    }
}

void Lexer::skipLineComment() {
    while (peek() != '\n' && !isAtEnd()) {
        advance();
    }
}

void Lexer::addToken(TokenType type) {
    addToken(type, std::monostate{});
}

void Lexer::addToken(TokenType type, std::variant<std::monostate, double, std::string> literal) {
    std::string lexeme = m_source.substr(m_startIdx, m_currentIdx - m_startIdx);
    m_tokens.emplace_back(type, std::move(lexeme), std::move(literal), m_line);
}

bool Lexer::match(char expected) {
    if (isAtEnd() || m_source[m_currentIdx] != expected)
        return false;
    m_currentIdx++;
    return true;
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : m_source[m_currentIdx];
}

bool Lexer::peekNext() const {
    if (m_currentIdx + 1 >= m_source.size()) return false;
    return isDigit(m_source[m_currentIdx + 1]);
}

void Lexer::runtimeErrorUnexpectedChar(char singleChar) {
    throw std::runtime_error(
        "[라인 " + std::to_string(m_line)
        + "] 어휘 오류: 인식할 수 없는 문자 '" + std::string(1, singleChar) + "'");
}
