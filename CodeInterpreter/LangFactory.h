#pragma once
#include <memory>
#include <string>
#include "ILexer.h"
#include "IParser.h"
#include "IChecker.h"
#include "IInterpreter.h"
#include "IOptimizer.h"

class LangFactory {
public:
    LangFactory();
    LangFactory(std::unique_ptr<ILexer>       lexer,
                std::unique_ptr<IParser>      parser,
                std::unique_ptr<IChecker>     checker,
                std::unique_ptr<IInterpreter> interpreter);

    // 최적화 패스 선택적 등록 (nullptr이면 스킵)
    void setOptimizer(std::unique_ptr<IOptimizer> optimizer);

    void run(const std::string& source);

private:
    std::unique_ptr<ILexer>       m_lexer;
    std::unique_ptr<IParser>      m_parser;
    std::unique_ptr<IOptimizer>   m_optimizer;   // optional
    std::unique_ptr<IChecker>     m_checker;
    std::unique_ptr<IInterpreter> m_interpreter;
};
