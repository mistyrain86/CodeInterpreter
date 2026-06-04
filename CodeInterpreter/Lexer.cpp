#include "Lexer.h"

std::vector<Token> Lexer::tokenize(const std::string& source) {
    m_tokens.emplace_back(TokenType::END_OF_FILE, "", std::monostate{}, 1);
    return m_tokens;
}
