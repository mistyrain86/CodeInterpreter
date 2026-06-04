#include "Lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& source) {
    if (source[0] == '(') {
        m_tokens.emplace_back(TokenType::LEFT_PAREN, "(", std::monostate{}, m_line);
    }

    m_tokens.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, m_line);
    return m_tokens;
}
