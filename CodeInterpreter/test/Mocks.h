#pragma once
#include <gmock/gmock.h>
#include "ILexer.h"
#include "IParser.h"
#include "IChecker.h"
#include "IInterpreter.h"

class MockLexer : public ILexer {
public:
    MOCK_METHOD(std::vector<Token>, tokenize,
                (const std::string&), (override));
};

class MockParser : public IParser {
public:
    MOCK_METHOD(std::vector<StmtPtr>, parse,
                (std::vector<Token>), (override));
};

class MockChecker : public IChecker {
public:
    MOCK_METHOD(void, check,
                (const std::vector<StmtPtr>&), (override));
};

class MockInterpreter : public IInterpreter {
public:
    MOCK_METHOD(void, interpret,
                (const std::vector<StmtPtr>&), (override));
};
