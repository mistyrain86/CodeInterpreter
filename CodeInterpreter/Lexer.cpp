#include "Lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& source) {
    for (char singleChar : source) {
        switch (singleChar) {
            case '(': addToken(TokenType::LEFT_PAREN, "(");  break;
            case ')': addToken(TokenType::RIGHT_PAREN, ")"); break;
            case '{': addToken(TokenType::LEFT_BRACE, "{");  break;
            case '}': addToken(TokenType::RIGHT_BRACE, "}"); break;
            case ';': addToken(TokenType::SEMICOLON, ";");   break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n': m_line++; break;
            default: break;
        }
    }

    addToken(TokenType::END_OF_FILE, "");
    return m_tokens;
}

void Lexer::addToken(TokenType type, std::string singleChar) {
    m_tokens.emplace_back(type, std::move(singleChar), std::monostate{}, m_line);
}