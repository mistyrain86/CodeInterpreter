#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ILexer.h"
#include "IParser.h"
#include "IChecker.h"
#include "IInterpreter.h"
#include "IOptimizer.h"
#include "BindingMap.h"
#include "Stmt.h"

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

    IInterpreter* getInterpreter() const { return m_interpreter.get(); }

private:
    std::unique_ptr<ILexer>       m_lexer;
    std::unique_ptr<IParser>      m_parser;
    std::unique_ptr<IOptimizer>   m_optimizer;
    std::unique_ptr<IChecker>     m_checker;
    std::unique_ptr<IInterpreter> m_interpreter;
    // LangFunction이 FunctionStmt&를 참조하므로 AST를 세션 내내 소유
    std::vector<std::vector<StmtPtr>> m_stmtHistory;
};
