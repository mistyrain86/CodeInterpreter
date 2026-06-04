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

private:
    std::unordered_map<std::string, Value> m_values;
    std::shared_ptr<Environment>           m_enclosing;
    std::string undefinedVarError(const Token& name) const;
};
