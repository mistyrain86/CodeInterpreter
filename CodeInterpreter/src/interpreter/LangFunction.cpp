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
    try {
        interp.executeBlock(m_decl.m_body, std::move(env));
    } catch (ReturnSignal& ret) {
        return ret.value;
    }
    return Value{std::monostate{}};  // return 없으면 null
}
