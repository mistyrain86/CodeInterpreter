#include "Environment.h"

Environment::Environment(std::shared_ptr<Environment> enc)
    : m_enclosing(std::move(enc)) {}

void Environment::define(const std::string& name, Value value) {
    m_values[name] = std::move(value);
}

Value Environment::get(const Token& name) const {
    auto it = m_values.find(name.lexeme);
    if (it != m_values.end()) return it->second;
    if (m_enclosing)           return m_enclosing->get(name);
    throw RuntimeError(undefinedVarError(name));
}

void Environment::assign(const Token& name, Value value) {
    auto it = m_values.find(name.lexeme);
    if (it != m_values.end()) { it->second = std::move(value); return; }
    if (m_enclosing) { m_enclosing->assign(name, std::move(value)); return; }
    throw RuntimeError(undefinedVarError(name));
}

std::string Environment::undefinedVarError(const Token& name) const {
    return "[라인 " + std::to_string(name.line)
        + "] 런타임 오류: 미정의된 변수 '" + name.lexeme + "'.";
}
