#pragma once
#include <vector>
#include "Stmt.h"

// 실행 전 AST 최적화 패스 인터페이스
// Lexer → Parser → [IOptimizer?] → Checker → Interpreter
class IOptimizer {
public:
    virtual ~IOptimizer() = default;
    virtual std::vector<StmtPtr> optimize(std::vector<StmtPtr> stmts) = 0;
};
