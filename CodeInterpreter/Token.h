#pragma once

#include <string>
#include <variant>

enum class TokenType {
    // 구분자
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    SEMICOLON,

    // 산술 연산자
    PLUS, MINUS, STAR, SLASH,

    // 비교 / 논리 연산자
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,

    // 리터럴
    IDENTIFIER, STRING, NUMBER,

    // 키워드
    KW_VAR, KW_PRINT,
    KW_IF, KW_ELSE,
    KW_FOR,
    KW_TRUE, KW_FALSE,

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