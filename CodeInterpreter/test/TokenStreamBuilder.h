#pragma once
#include <string>
#include <variant>
#include <vector>
#include "Token.h"

class TokenStreamBuilder {
public:
    TokenStreamBuilder& number(double v, int line = 1) {
        return add(TokenType::NUMBER, std::to_string(v), v, line);
    }
    TokenStreamBuilder& string(std::string v, int line = 1) {
        std::string lex = "\"" + v + "\"";
        return add(TokenType::STRING, std::move(lex), std::move(v), line);
    }
    TokenStreamBuilder& boolTrue (int line = 1) { return add(TokenType::KW_TRUE,  "true",  {}, line); }
    TokenStreamBuilder& boolFalse(int line = 1) { return add(TokenType::KW_FALSE, "false", {}, line); }

    TokenStreamBuilder& identifier(std::string name, int line = 1) {
        return add(TokenType::IDENTIFIER, std::move(name), {}, line);
    }

    TokenStreamBuilder& kwVar   (int line = 1) { return add(TokenType::KW_VAR,    "var",    {}, line); }
    TokenStreamBuilder& kwPrint (int line = 1) { return add(TokenType::KW_PRINT,  "print",  {}, line); }
    TokenStreamBuilder& kwIf    (int line = 1) { return add(TokenType::KW_IF,     "if",     {}, line); }
    TokenStreamBuilder& kwElse  (int line = 1) { return add(TokenType::KW_ELSE,   "else",   {}, line); }
    TokenStreamBuilder& kwFor   (int line = 1) { return add(TokenType::KW_FOR,    "for",    {}, line); }
    TokenStreamBuilder& kwFunc  (int line = 1) { return add(TokenType::KW_FUNC,   "func",   {}, line); }
    TokenStreamBuilder& kwReturn(int line = 1) { return add(TokenType::KW_RETURN, "return", {}, line); }

    TokenStreamBuilder& plus (int line = 1) { return add(TokenType::PLUS,  "+", {}, line); }
    TokenStreamBuilder& minus(int line = 1) { return add(TokenType::MINUS, "-", {}, line); }
    TokenStreamBuilder& star (int line = 1) { return add(TokenType::STAR,  "*", {}, line); }
    TokenStreamBuilder& slash(int line = 1) { return add(TokenType::SLASH, "/", {}, line); }

    TokenStreamBuilder& bang        (int line = 1) { return add(TokenType::BANG,          "!",  {}, line); }
    TokenStreamBuilder& bangEqual   (int line = 1) { return add(TokenType::BANG_EQUAL,    "!=", {}, line); }
    TokenStreamBuilder& equal       (int line = 1) { return add(TokenType::EQUAL,         "=",  {}, line); }
    TokenStreamBuilder& equalEqual  (int line = 1) { return add(TokenType::EQUAL_EQUAL,   "==", {}, line); }
    TokenStreamBuilder& less        (int line = 1) { return add(TokenType::LESS,          "<",  {}, line); }
    TokenStreamBuilder& lessEqual   (int line = 1) { return add(TokenType::LESS_EQUAL,    "<=", {}, line); }
    TokenStreamBuilder& greater     (int line = 1) { return add(TokenType::GREATER,       ">",  {}, line); }
    TokenStreamBuilder& greaterEqual(int line = 1) { return add(TokenType::GREATER_EQUAL, ">=", {}, line); }

    TokenStreamBuilder& semicolon(int line = 1) { return add(TokenType::SEMICOLON,    ";", {}, line); }
    TokenStreamBuilder& comma    (int line = 1) { return add(TokenType::COMMA,        ",", {}, line); }
    TokenStreamBuilder& lparen   (int line = 1) { return add(TokenType::LEFT_PAREN,   "(", {}, line); }
    TokenStreamBuilder& rparen   (int line = 1) { return add(TokenType::RIGHT_PAREN,  ")", {}, line); }
    TokenStreamBuilder& lbrace   (int line = 1) { return add(TokenType::LEFT_BRACE,   "{", {}, line); }
    TokenStreamBuilder& rbrace   (int line = 1) { return add(TokenType::RIGHT_BRACE,  "}", {}, line); }
    TokenStreamBuilder& lbracket (int line = 1) { return add(TokenType::LEFT_BRACKET, "[", {}, line); }
    TokenStreamBuilder& rbracket (int line = 1) { return add(TokenType::RIGHT_BRACKET,"]", {}, line); }
    TokenStreamBuilder& eof      (int line = 1) { return add(TokenType::END_OF_FILE,  "",  {}, line); }

    std::vector<Token> build() const { return m_tokens; }

private:
    std::vector<Token> m_tokens;

    TokenStreamBuilder& add(TokenType type, std::string lex,
                             std::variant<std::monostate, double, std::string> lit,
                             int line) {
        m_tokens.push_back(Token{type, std::move(lex), std::move(lit), line});
        return *this;
    }
};
