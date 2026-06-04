#include "LangFactory.h"
// NOTE: 실제 구현체는 각 팀원 작업 완료 후 include 추가
// 현재는 DI 생성자만 동작

LangFactory::LangFactory() {
    // TODO: A,B,C,D 완료 후
    // m_lexer        = std::make_unique<Lexer>();
    // m_parser       = std::make_unique<Parser>();
    // m_checker      = std::make_unique<Checker>();
    // m_interpreter  = std::make_unique<Interpreter>();
}

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
