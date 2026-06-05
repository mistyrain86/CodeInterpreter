#pragma once
#include "ICallable.h"

// Ch.3: Array(n) 내장 함수 — ICallable 구현체
// D가 ArrayBuiltin.cpp 에서 구현
class ArrayBuiltin : public ICallable {
public:
    int         arity() const override { return 1; }
    std::string name()  const override { return "Array"; }
    Value       call(Interpreter& interp, const std::vector<Value>& args) override;
};
