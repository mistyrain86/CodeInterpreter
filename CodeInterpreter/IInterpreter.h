#pragma once
#include <vector>
#include "Stmt.h"

class IInterpreter {
public:
    virtual ~IInterpreter() = default;
    virtual void interpret(const std::vector<StmtPtr>& stmts) = 0;
};
