#pragma once
#include "ICallable.h"

class ArrayBuiltin : public ICallable {
public:
    int         arity() const override { return 1; }
    std::string name()  const override { return "Array"; }
    Value       call(Interpreter& interp, const std::vector<Value>& args) override;
};
