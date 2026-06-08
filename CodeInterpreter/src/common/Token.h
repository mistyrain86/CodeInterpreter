#pragma once

#include <string>
#include <variant>

enum class TokenType {
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET,
    SEMICOLON,
    COMMA,

    PLUS, MINUS, STAR, SLASH, PERCENT,

    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,

    IDENTIFIER, STRING, NUMBER,

    KW_VAR, KW_PRINT,
    KW_IF, KW_ELSE,
    KW_FOR,
    KW_TRUE, KW_FALSE,
    KW_FUNC, KW_RETURN,

    END_OF_FILE
};

struct Token {
    TokenType                                          type;
    std::string                                        lexeme;
    std::variant<std::monostate, double, std::string>  literal;
    int                                                line;

    Token(TokenType type, std::string lexeme,
        std::variant<std::monostate, double, std::string> literal,
        int line)
        : type(type)
        , lexeme(std::move(lexeme))
        , literal(std::move(literal))
        , line(line) {
    }
};