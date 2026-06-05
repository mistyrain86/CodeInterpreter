#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ICallable.h"
#include "Stmt.h"
#include "Environment.h"

class Interpreter;

// Ch.2: 사용자 정의 함수 — ICallable 구현체
// D가 LangFunction.cpp 에서 구현
class LangFunction : public ICallable {
public:
    LangFunction(FunctionStmt& decl, std::shared_ptr<Environment> closure)
        : m_decl(decl), m_closure(std::move(closure)) {}

    int         arity() const override;
    Value       call(Interpreter& interp, const std::vector<Value>& args) override;
    std::string name()  const override;

private:
    FunctionStmt&                m_decl;
    std::shared_ptr<Environment> m_closure;
};
