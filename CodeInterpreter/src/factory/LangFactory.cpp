#include "LangFactory.h"
#include "Lexer.h"
#include "Parser.h"
#include "Checker.h"
#include "Interpreter.h"
#include "Resolver.h"
#include "ConstantFolder.h"
#include <iostream>

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
    auto stmts  = m_parser->parse(std::move(tokens));   // 에러 시 throw (기존 동작)
    if (m_optimizer) stmts = m_optimizer->optimize(std::move(stmts));
    return stmts;
}

void LangFactory::runWithRecovery(const std::string& source) {
    auto* p = dynamic_cast<Parser*>(m_parser.get());
    if (!p) { run(source); return; }   // 구체 Parser 없으면 일반 실행

    // 토큰을 한 번만 렉싱해 파서에 로드 — 이후 statement 단위로 순차 처리
    p->prepare(m_lexer->tokenize(source));

    while (p->hasMore()) {
        // ── 1. statement 하나 파싱 ──────────────────────────────
        StmtPtr stmt;
        try {
            stmt = p->parseOne();
        } catch (const ParseError& e) {
            std::cerr << "[구문 오류] " << e.what() << "\n";
            return;   // 요구사항 3: 즉시 종료
        }

        std::vector<StmtPtr> single;
        single.push_back(std::move(stmt));
        if (m_optimizer) single = m_optimizer->optimize(std::move(single));

        // ── 2. 의미 검사 ────────────────────────────────────────
        try {
            bindAndCheck(single);
        } catch (const CheckError& e) {
            std::cerr << "[의미 오류] " << e.what() << "\n";
            return;   // 즉시 종료
        }

        // ── 3. 실행 — AST를 history에 먼저 이동해 LangFunction 참조 유지 ──
        m_stmtHistory.push_back(std::move(single));
        try {
            m_interpreter->interpret(m_stmtHistory.back());
            syncGlobals();
        } catch (const RuntimeError& e) {
            std::cerr << "[런타임 오류] " << e.what() << "\n";
            return;   // 요구사항 2+3: 줄 번호 포함 출력 후 즉시 종료
        } catch (const std::runtime_error& e) {
            std::cerr << "[오류] " << e.what() << "\n";
            return;
        }
    }
}

void LangFactory::bindAndCheck(const std::vector<StmtPtr>& stmts) {
    Resolver resolver;
    BindingMap bindings = resolver.resolve(stmts);
    m_interpreter->setBindings(&bindings);
    struct Guard {
        IInterpreter* p;
        ~Guard() { p->setBindings(nullptr); }
    } guard{ m_interpreter.get() };
    m_checker->check(stmts);
}

void LangFactory::syncGlobals() {
    for (const auto& name : m_interpreter->globalNames())
        m_checker->registerGlobal(name);
}
