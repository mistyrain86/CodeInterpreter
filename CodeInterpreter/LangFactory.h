#pragma once
#include <memory>
#include <string>
#include "ILexer.h"
#include "IParser.h"
#include "IChecker.h"
#include "IInterpreter.h"

class LangFactory {
public:
    LangFactory() = default;
    LangFactory(std::unique_ptr<ILexer>       lexer,
                std::unique_ptr<IParser>      parser,
                std::unique_ptr<IChecker>     checker,
                std::unique_ptr<IInterpreter> interpreter);

    void run(const std::string& source);

private:
    std::unique_ptr<ILexer>       m_lexer;
    std::unique_ptr<IParser>      m_parser;
    std::unique_ptr<IChecker>     m_checker;
    std::unique_ptr<IInterpreter> m_interpreter;
};
