#include "LangFactory.h"


LangFactory::LangFactory(std::unique_ptr<ILexer>       lexer,
                         std::unique_ptr<IParser>      parser,
                         std::unique_ptr<IChecker>     checker,
                         std::unique_ptr<IInterpreter> interpreter)
    : m_lexer(std::move(lexer))
    , m_parser(std::move(parser))
    , m_checker(std::move(checker))
    , m_interpreter(std::move(interpreter)) {}

void LangFactory::run(const std::string& source) {
    auto tokens = m_lexer->tokenize(source);
    auto stmts  = m_parser->parse(std::move(tokens));
    m_checker->check(stmts);
    m_interpreter->interpret(stmts);
}
