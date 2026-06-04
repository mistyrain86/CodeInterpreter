#include "Lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& source) {
    m_tokens.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, m_line);
    return m_tokens;
}
