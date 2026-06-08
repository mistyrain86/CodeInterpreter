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

    void setOptimizer(std::unique_ptr<IOptimizer> optimizer);

    void run(const std::string& source);
    void runWithRecovery(const std::string& source);  // statement별 실행, 에러 무시하고 계속

    IInterpreter* getInterpreter() const { return m_interpreter.get(); }

private:
    std::unique_ptr<ILexer>       m_lexer;
    std::unique_ptr<IParser>      m_parser;
    std::unique_ptr<IOptimizer>   m_optimizer;
    std::unique_ptr<IChecker>     m_checker;
    std::unique_ptr<IInterpreter> m_interpreter;
    std::vector<std::vector<StmtPtr>> m_stmtHistory;

    std::vector<StmtPtr> compileToAst(const std::string& source);
    void                 bindAndCheck(const std::vector<StmtPtr>& stmts);
    void                 syncGlobals();
};
