#include "Lexer.h"

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
    case '/': addToken(TokenType::SLASH);       break;
    case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);    break;
    case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);   break;
    case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);    break;
    case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n': m_line++; break;
    default: break;
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
    if (isAtEnd() || checkNextChar(expected))
        return false;
    m_currentIdx++;
    return true;
}

bool Lexer::checkNextChar(char expected)
{
    return m_source[m_currentIdx] != expected;
}
