#include "Lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& source) {
    if (source[0] == '(') {
        addToken(TokenType::LEFT_PAREN, "(");
    }

    addToken(TokenType::END_OF_FILE, "");
    return m_tokens;
}

void Lexer::addToken(TokenType type, std::string singleChar) {
    m_tokens.emplace_back(type, std::move(singleChar), std::monostate{}, m_line);
}