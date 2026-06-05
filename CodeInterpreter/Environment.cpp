#include "Environment.h"
#include <iostream>

Environment::Environment(std::shared_ptr<Environment> enc)
    : m_enclosing(std::move(enc)) {}

void Environment::define(const std::string& name, Value value) {
    m_values[name] = std::move(value);
}

Value Environment::get(const Token& name) const {
    auto it = m_values.find(name.lexeme);
    if (it != m_values.end()) return it->second;
    if (m_enclosing)           return m_enclosing->get(name);
    throw RuntimeError(makeUndefinedVarMessage(name));
}

void Environment::assign(const Token& name, Value value) {
    auto it = m_values.find(name.lexeme);
    if (it != m_values.end()) { it->second = std::move(value); return; }
    if (m_enclosing) { m_enclosing->assign(name, std::move(value)); return; }
    throw RuntimeError(makeUndefinedVarMessage(name));
}

void Environment::assignOrDefine(const Token& name, Value value) {
    auto it = m_values.find(name.lexeme);
    if (it != m_values.end()) { it->second = std::move(value); return; }
    if (m_enclosing) { m_enclosing->assignOrDefine(name, std::move(value)); return; }
    // 전역 스코프까지 탐색 후 없으면 전역에 암묵적 선언
    m_values[name.lexeme] = std::move(value);
}

Value Environment::getAt(int distance, const std::string& name) const {
    const Environment* env = this;
    for (int i = 0; i < distance; i++) env = env->m_enclosing.get();
    return env->m_values.at(name);
}

void Environment::assignAt(int distance, const std::string& name, Value value) {
    Environment* env = this;
    for (int i = 0; i < distance; i++) env = env->m_enclosing.get();
    env->m_values[name] = std::move(value);
}

void Environment::printAll(int depth) const {
    if (m_enclosing) m_enclosing->printAll(depth + 1);
    std::string scope = (depth == 0) ? "[전역]" : "[로컬]";
    for (auto& [k, v] : m_values)
        std::cout << scope << " " << k << "\n";  // D의 stringify 호출로 교체 가능
}

std::string Environment::makeUndefinedVarMessage(const Token& name) const {
    return "[라인 " + std::to_string(name.line)
        + "] 런타임 오류: 미정의된 변수 '" + name.lexeme + "'.";
}
