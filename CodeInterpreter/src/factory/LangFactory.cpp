#include "LangFactory.h"
#include "Lexer.h"
#include "Parser.h"
#include "Checker.h"
#include "Interpreter.h"
#include "Resolver.h"
#include "ConstantFolder.h"

LangFactory::LangFactory()
    : m_lexer       (std::make_unique<Lexer>())
    , m_parser      (std::make_unique<Parser>())
    , m_checker     (std::make_unique<Checker>())
    , m_interpreter (std::make_unique<Interpreter>()) {}

LangFactory::LangFactory(std::unique_ptr<ILexer>       lexer,
                         std::unique_ptr<IParser>      parser,
                         std::unique_ptr<IChecker>     checker,
                         std::unique_ptr<IInterpreter> interpreter)
    : m_lexer(std::move(lexer))
    , m_parser(std::move(parser))
    , m_checker(std::move(checker))
    , m_interpreter(std::move(interpreter)) {}

void LangFactory::setOptimizer(std::unique_ptr<IOptimizer> optimizer) {
    m_optimizer = std::move(optimizer);
}

void LangFactory::run(const std::string& source) {
    auto stmts = compileToAst(source);
    bindAndCheck(stmts);
    m_interpreter->interpret(stmts);
    syncGlobals();
    m_stmtHistory.push_back(std::move(stmts));
}

std::vector<StmtPtr> LangFactory::compileToAst(const std::string& source) {
    auto tokens = m_lexer->tokenize(source);
    auto stmts  = m_parser->parse(std::move(tokens));
    if (m_optimizer) stmts = m_optimizer->optimize(std::move(stmts));
    return stmts;
}

void LangFactory::bindAndCheck(const std::vector<StmtPtr>& stmts) {
    Resolver resolver;
    BindingMap bindings = resolver.resolve(stmts);
    m_interpreter->setBindings(&bindings);
    m_checker->check(stmts);
    m_interpreter->setBindings(nullptr);
}

void LangFactory::syncGlobals() {
    for (const auto& name : m_interpreter->globalNames())
        m_checker->registerGlobal(name);
}
