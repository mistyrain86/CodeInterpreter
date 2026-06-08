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
    auto tokens = m_lexer->tokenize(source);
    auto stmts  = m_parser->parse(std::move(tokens));

    if (m_optimizer)
        stmts = m_optimizer->optimize(std::move(stmts));

    Resolver resolver;
    BindingMap bindings = resolver.resolve(stmts);
    auto* interp = dynamic_cast<Interpreter*>(m_interpreter.get());
    if (interp) interp->setBindings(&bindings);

    m_checker->check(stmts);
    m_interpreter->interpret(stmts);

    if (interp) interp->setBindings(nullptr);

    if (interp)
        if (auto* chk = dynamic_cast<Checker*>(m_checker.get()))
            for (const auto& name : interp->globalNames())
                chk->registerGlobal(name);

    // LangFunction이 FunctionStmt&를 참조하므로 AST 소유권을 유지
    m_stmtHistory.push_back(std::move(stmts));
}
