#pragma once
#include <memory>
#include <stdexcept>
#include "IParser.h"
#include "ParseError.h"

class ParserProxy : public IParser {
public:
    explicit ParserProxy(std::unique_ptr<IParser> inner)
        : m_inner(std::move(inner)) {}

    std::vector<StmtPtr> parse(std::vector<Token> tokens) override {
        validate(tokens);
        return m_inner->parse(std::move(tokens));
    }

private:
    std::unique_ptr<IParser> m_inner;

    static void validate(const std::vector<Token>& tokens) {
        if (tokens.empty())
            throw ParseError("[파서] 오류: 토큰 배열이 비어 있습니다.");
    }
};
