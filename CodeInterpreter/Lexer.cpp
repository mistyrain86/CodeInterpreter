#include "Lexer.h"
#include <stdexcept>

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

bool Lexer::isAtEnd() const { return m_currentIdx >= m_source.size(); }

void Lexer::scanToken() {
    char singleChar = m_source[m_currentIdx++];
    switch (singleChar) {
    case '(': addToken(TokenType::LEFT_PAREN);  break;
    case ')': addToken(TokenType::RIGHT_PAREN); break;
    case '{': addToken(TokenType::LEFT_BRACE);  break;
    case '}': addToken(TokenType::RIGHT_BRACE); break;
    case ';': addToken(TokenType::SEMICOLON);   break;
    case '+': addToken(TokenType::PLUS);        break;
    case '-': addToken(TokenType::MINUS);       break;
    case '*': addToken(TokenType::STAR);        break;
    case '/': 
            if (match('/'))
                skipLineComment();
            else
                addToken(TokenType::SLASH);
            break;
    case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);    break;
    case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);   break;
    case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);    break;
    case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n': m_line++; break;
    case '"' : scanString(); break;
    default: 
            if (std::isdigit((unsigned char)singleChar)) scanNumber();
            else break;
    }
}

void Lexer::addToken(TokenType type) {
    std::string singleChar = m_source.substr(m_startIdx, m_currentIdx - m_startIdx);
    m_tokens.emplace_back(type, std::move(singleChar), std::monostate{}, m_line);
}

void Lexer::addToken(TokenType type,
    std::variant<std::monostate, double, std::string> literal) {
    std::string lex = m_source.substr(m_startIdx, m_currentIdx - m_startIdx);
    m_tokens.emplace_back(type, std::move(lex), std::move(literal), m_line);
}

bool Lexer::match(char expected) {
    if (isAtEnd() || !isNextChar(expected))
        return false;
    m_currentIdx++;
    return true;
}

bool Lexer::isNextChar(char expected)
{
    return m_source[m_currentIdx] == expected;
}

void Lexer::skipLineComment()
{
    while (peek() != '\n' && !isAtEnd())
        m_currentIdx++;
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : m_source[m_currentIdx];
}

void Lexer::scanString() {
    advanceToClosingQuote();

    m_currentIdx++;
    std::string val = m_source.substr(m_startIdx + 1, m_currentIdx - m_startIdx - 2);
    addToken(TokenType::STRING, std::move(val));
}

void Lexer::advanceToClosingQuote()
{
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            m_line++;
        m_currentIdx++;
    }

    if (isAtEnd()) {
        throw std::runtime_error(
            "[라인 " + std::to_string(m_line) + "] 어휘 오류: 문자열이 닫히지 않았습니다.");
    }

}

void Lexer::scanNumber() {
    advanceDigits();

    if (peek() == '.' && peekNext()) {
        m_currentIdx++;

        advanceDigits();
    }

    double val = std::stod(m_source.substr(m_startIdx, m_currentIdx - m_startIdx));
    addToken(TokenType::NUMBER, val);
}

void Lexer::advanceDigits()
{
    while (std::isdigit((unsigned char)peek()))
        m_currentIdx++;
}

bool Lexer::peekNext() const {
    char nextChar = (m_currentIdx + 1 >= m_source.size()) ? '\0' : m_source[m_currentIdx + 1];
    return std::isdigit((unsigned char)nextChar);
}