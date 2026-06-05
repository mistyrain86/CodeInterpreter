#include "LangFunction.h"
#include "Interpreter.h"
#include "ReturnSignal.h"

int LangFunction::arity() const {
    return (int)m_decl.params.size();
}

std::string LangFunction::name() const {
    return m_decl.name.lexeme;
}

Value LangFunction::call(Interpreter& interp, const std::vector<Value>& args) {
    // 클로저 환경 기반 새 스코프 생성
    auto env = std::make_shared<Environment>(m_closure);
    // 파라미터 바인딩
    for (int i = 0; i < (int)m_decl.params.size(); i++)
        env->define(m_decl.params[i].lexeme, args[i]);
    // 본문 실행 — ReturnSignal catch → 반환값 추출
    try {
        interp.executeBlock(m_decl.body, std::move(env));
    } catch (ReturnSignal& ret) {
        return ret.value;
    }
    return Value{std::monostate{}};  // return 없으면 nil
}
