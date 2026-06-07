#pragma once
#include <vector>
#include "Stmt.h"
#include "Token.h"

class IParser {
public:
    virtual ~IParser() = default;
    virtual std::vector<StmtPtr> parse(std::vector<Token> tokens) = 0;
};
