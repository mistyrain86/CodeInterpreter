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
    std::string        m_source;
    int                m_line = 1;
    std::size_t        m_startIdx = 0;
    std::size_t        m_currentIdx = 0;

    void reset(const std::string& source);
    bool isAtEnd() const;
    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, std::variant<std::monostate, double, std::string> literal);

    bool match(char expected);
    bool checkNextChar(char expected);
    char peek() const;
};
