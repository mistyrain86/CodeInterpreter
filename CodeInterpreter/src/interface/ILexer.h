#pragma once
#include <string>
#include <vector>
#include "Token.h"

class ILexer {
public:
    virtual ~ILexer() = default;
    virtual std::vector<Token> tokenize(const std::string& source) = 0;
};
