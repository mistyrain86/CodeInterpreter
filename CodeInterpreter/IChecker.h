#pragma once
#include <vector>
#include "Stmt.h"

class IChecker {
public:
    virtual ~IChecker() = default;
    virtual void check(const std::vector<StmtPtr>& stmts) = 0;
};
