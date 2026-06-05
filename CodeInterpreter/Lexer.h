#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
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

    static const std::unordered_map<std::string, TokenType> s_keywords;

    void reset(const std::string& source);
    bool isAtEnd() const;

    void scanToken();
    bool isWhitespace(char c) const;
    bool isDigit(char c) const;
    bool isAlphaOrUnderscore(char c) const;
    bool scanPunctuatorAndOperator(char singleChar);
    void scanString();
    void scanNumber();
    void scanIdentifier();

    char advance();
    void advanceDigits();
    void skipLineComment();

    void addToken(TokenType type);
    void addToken(TokenType type, std::variant<std::monostate, double, std::string> literal);

    bool match(char expected);
    char peek() const;
    bool peekNext() const;

    void runtimeErrorUnexpectedChar(char singleChar);
};
