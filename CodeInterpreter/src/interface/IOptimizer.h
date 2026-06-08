#pragma once
#include <vector>
#include "Stmt.h"

class IOptimizer {
public:
    virtual ~IOptimizer() = default;
    virtual std::vector<StmtPtr> optimize(std::vector<StmtPtr> stmts) = 0;
};
