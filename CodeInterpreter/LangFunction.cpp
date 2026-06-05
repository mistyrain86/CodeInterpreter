#include "LangFunction.h"
#include "Interpreter.h"
#include "ReturnSignal.h"

// TODO (D): 아래 3개 메서드를 구현하세요.

int LangFunction::arity() const {
    return (int)m_decl.params.size();
}

std::string LangFunction::name() const {
    return m_decl.name.lexeme;
}

Value LangFunction::call(Interpreter& interp, const std::vector<Value>& args) {
    // 1. 클로저 환경 기반 새 스코프 생성
    // 2. 파라미터 바인딩
    // 3. 본문 실행 (ReturnSignal catch)
    // 4. 반환값 없으면 nil 반환
    throw RuntimeError("미구현: LangFunction::call()");
}
