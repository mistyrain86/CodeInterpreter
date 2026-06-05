#pragma once
#include <memory>
#include <stdexcept>
#include "IParser.h"
#include "ParseError.h"

// ── Proxy 패턴 ─────────────────────────────────────────────
// 실제 Parser(RealSubject) 앞에 위치해 사전/사후 처리를 담당한다.
// IParser 인터페이스를 그대로 구현하므로 호출자는 변경 없이 사용 가능.
//
// 현재 제공 기능:
//   - 빈 토큰 배열 사전 검증 (ParseError throw)
//   - 파싱 결과 후처리 (예: 결과 검증 확장 포인트)
//
// 향후 확장 예시:
//   - 파싱 시간 측정 / 로깅
//   - 토큰 배열 전처리 (전처리 토큰 삽입 등)
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
