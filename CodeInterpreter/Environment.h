#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "RuntimeError.h"
#include "Token.h"
#include "Value.h"

class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> enclosing = nullptr);
    void  define(const std::string& name, Value value);
    Value get(const Token& name) const;
    void  assign(const Token& name, Value value);

    // Ch.4 정적 바인딩 — distance 기반 즉시 접근
    Value getAt(int distance, const std::string& name) const;
    void  assignAt(int distance, const std::string& name, Value value);

    // Ch.5 디버거 inspect — 스코프 전체 출력
    void printAll(int depth = 0) const;

// Environment에서 직접 접근 가능하도록 friend 허용
friend class Interpreter;
friend class Debugger;

private:
    std::unordered_map<std::string, Value> m_values;
    std::shared_ptr<Environment>           m_enclosing;
    std::string makeUndefinedVarMessage(const Token& name) const;
};
