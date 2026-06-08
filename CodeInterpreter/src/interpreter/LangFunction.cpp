#include "LangFunction.h"
#include "Interpreter.h"
#include "ReturnSignal.h"

int LangFunction::arity() const {
    return (int)m_decl.m_params.size();
}

std::string LangFunction::name() const {
    return m_decl.m_name.lexeme;
}

Value LangFunction::call(Interpreter& interp, const std::vector<Value>& args) {
    auto env = std::make_shared<Environment>(m_closure);
    for (int i = 0; i < (int)m_decl.m_params.size(); i++)
        env->define(m_decl.m_params[i].lexeme, args[i]);

    // 클로저 체인이 현재 scope stack과 다를 수 있으므로 재구성
    // (executeBlock이 funcEnv를 push하기 전에 closure 기준으로 맞춤)
    auto savedStack = interp.saveScopeStack();
    interp.rebuildScopeStackFromClosure(m_closure.get());

    try {
        interp.executeBlock(m_decl.m_body, std::move(env));
    } catch (ReturnSignal& ret) {
        interp.restoreScopeStack(std::move(savedStack));
        return ret.value;
    }
    interp.restoreScopeStack(std::move(savedStack));
    return Value{std::monostate{}};
}
