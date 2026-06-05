#pragma once
#include <initializer_list>
#include <string>
#include <vector>
#include "ParseError.h"
#include "Token.h"

// ── Adapter 패턴 ───────────────────────────────────────────
// std::vector<Token>(Adaptee)를 Parser가 요구하는
// 커서(cursor) 인터페이스(Target)로 변환한다.
// Parser는 인덱스 산술 없이 문법 분석 로직에만 집중할 수 있다.
class TokenStream {
public:
    TokenStream() = default;

    void load(std::vector<Token> tokens) {
        m_tokens  = std::move(tokens);
        m_current = 0;
    }

    bool isAtEnd() const {
        return peek().type == TokenType::END_OF_FILE;
    }

    const Token& peek()     const { return m_tokens[m_current]; }
    const Token& previous() const { return m_tokens[m_current - 1]; }

    const Token& advance() {
        if (!isAtEnd()) ++m_current;
        return previous();
    }

    bool check(TokenType t) const {
        return !isAtEnd() && peek().type == t;
    }

    bool match(std::initializer_list<TokenType> types) {
        for (auto t : types) {
            if (check(t)) { advance(); return true; }
        }
        return false;
    }

    const Token& consume(TokenType t, const std::string& msg) {
        if (check(t)) return advance();
        throw error(peek(), msg);
    }

    ParseError error(const Token& tok, const std::string& msg) const {
        std::string loc = (tok.type == TokenType::END_OF_FILE)
            ? " (파일 끝)"
            : " ('" + tok.lexeme + "' 근처)";
        return ParseError("[라인 " + std::to_string(tok.line)
                          + "] 구문 오류: " + msg + loc);
    }

private:
    std::vector<Token> m_tokens;
    int                m_current = 0;
};
