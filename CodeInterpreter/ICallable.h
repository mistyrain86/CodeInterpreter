#pragma once
#include <string>
#include <vector>

class Interpreter;
struct Value;

class ICallable {
public:
    virtual ~ICallable() = default;
    virtual int   arity() const = 0;
    virtual Value call(Interpreter& interp, const std::vector<Value>& args) = 0;
    virtual std::string name() const = 0;
};
