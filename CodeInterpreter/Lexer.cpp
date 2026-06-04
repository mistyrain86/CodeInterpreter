#include "Lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& source) {
    reset(source);

    while (m_currentIdx < m_source.size()) {
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

void Lexer::scanToken() {
    char singleChar = m_source[m_currentIdx++];
    switch (singleChar) {
        case '(': addToken(TokenType::LEFT_PAREN);  break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE);  break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ';': addToken(TokenType::SEMICOLON);   break;
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
