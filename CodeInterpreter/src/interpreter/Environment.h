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

    Value getAt(int distance, const std::string& name) const;
    void  assignAt(int distance, const std::string& name, Value value);
    void  printAll(int depth = 0) const;

    // 테스트 검증용: get()에서 현재 스코프에 없어 상위로 이동한 횟수
    static void resetChainSteps() { s_chainSteps = 0; }
    static int  chainSteps()      { return s_chainSteps; }

    // 테스트 검증용: getAt()에서 포인터 순회(체인 이동)가 발생한 횟수
    static void resetGetAtHops() { s_getAtHops = 0; }
    static int  getAtHops()      { return s_getAtHops; }

// Environment에서 직접 접근 가능하도록 friend 허용
friend class Interpreter;
friend class Debugger;

private:
    std::unordered_map<std::string, Value> m_values;
    std::shared_ptr<Environment>           m_enclosing;
    std::string makeUndefinedVarMessage(const Token& name) const;

    static inline int s_chainSteps = 0;
    static inline int s_getAtHops  = 0;
};
