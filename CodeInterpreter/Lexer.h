#pragma once
#include <string>
#include <vector>
#include "ILexer.h"
#include "Token.h"

class Lexer : public ILexer {
public:
    Lexer() = default;
    std::vector<Token> tokenize(const std::string& source) override;

private:
    std::vector<Token> m_tokens;
    int                m_line = 1;

    void addToken(TokenType type, std::string singleChar);
};
