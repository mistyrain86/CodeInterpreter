#include <gtest/gtest.h>
#include "Lexer.h"
#include "Parser.h"
#include "Resolver.h"
#include "Interpreter.h"
#include "Environment.h"
#include "LangFactory.h"

// 공통 헬퍼: 소스 → 토큰 → AST → BindingMap
static std::pair<std::vector<StmtPtr>, BindingMap>
parseAndResolve(const std::string& source) {
    Lexer  lexer;
    Parser parser;
    auto tokens = lexer.tokenize(source);
    auto stmts  = parser.parse(std::move(tokens));
    Resolver resolver;
    auto bindings = resolver.resolve(stmts);
    return { std::move(stmts), std::move(bindings) };
}

// ── TC1: 정적 바인딩 활성화 ──────────────────────────────────────────
// scope stack O(1) 직접 접근 경로 사용 확인
// + get() 체인 탐색 없음 + getAt() 포인터 순회 없음
TEST(StaticBindingTest, NestedLocalVar_UsesDirectAccess) {
    const std::string source = R"(
{
    var a = 0;
    { { { { { { { { { { { { {
        a = a + 1;
    } } } } } } } } } } } } }
}
)";
    auto [stmts, bindings] = parseAndResolve(source);

    Interpreter::BindingSpy spy;
    Interpreter interp;
    interp.setBindings(&bindings);
    interp.setSpy(&spy);

    Environment::resetChainSteps();
    Environment::resetGetAtHops();
    interp.interpret(stmts);

    EXPECT_GT(spy.m_bindingHits, 0)
        << "scope stack O(1) 접근 경로가 호출돼야 한다";
    EXPECT_EQ(spy.m_chainWalks, 0)
        << "지역 변수 접근에 get 경로가 사용돼선 안 된다";
    EXPECT_EQ(Environment::chainSteps(), 0)
        << "get()에서 상위 스코프로 이동한 횟수가 0이어야 한다";
    EXPECT_EQ(Environment::getAtHops(), 0)
        << "getAt() 포인터 순회가 발생하지 않아야 한다 — scope stack이 getAt을 대체함";
}

// ── TC2: 정적 바인딩 비활성화 ─────────────────────────────────────────
// 바인딩 없이 실행 시 get() 체인 탐색 발생 확인
TEST(StaticBindingTest, WithoutBinding_UsesChainWalk) {
    const std::string source = R"(
{
    var a = 0;
    { { { { { { { { { { { { {
        a = a + 1;
    } } } } } } } } } } } } }
}
)";
    auto [stmts, bindings] = parseAndResolve(source);
    (void)bindings;

    Interpreter::BindingSpy spy;
    Interpreter interp;
    interp.setSpy(&spy);

    Environment::resetChainSteps();
    Environment::resetGetAtHops();
    interp.interpret(stmts);

    EXPECT_GT(spy.m_chainWalks, 0)
        << "바인딩 없이는 get 체인 탐색 경로가 사용돼야 한다";
    EXPECT_EQ(spy.m_bindingHits, 0)
        << "바인딩 없으면 scope stack 접근 경로가 사용되지 않아야 한다";
    EXPECT_GT(Environment::chainSteps(), 0)
        << "변수를 찾을 때까지 스코프 체인을 거슬러 올라가야 한다";
}

// ── TC3: O(1) depth 독립성 검증 ───────────────────────────────────────
// 정적 바인딩이 진정한 O(1)이라면 중첩 깊이에 관계없이
// 체인 순회 횟수는 항상 0이어야 한다
TEST(StaticBindingTest, O1_AccessIndependentOfDepth) {
    auto countTraversals = [](int depth) -> int {
        std::string open(depth, '{');
        std::string close(depth, '}');
        std::string source = "{ var a = 0; " + open + " a = a + 1; " + close + " }";

        auto [stmts, bindings] = parseAndResolve(source);
        Interpreter::BindingSpy spy;
        Interpreter interp;
        interp.setBindings(&bindings);
        interp.setSpy(&spy);

        Environment::resetChainSteps();
        Environment::resetGetAtHops();
        interp.interpret(stmts);

        return Environment::chainSteps() + Environment::getAtHops();
    };

    // 깊이가 달라져도 체인 순회 횟수는 항상 0
    EXPECT_EQ(countTraversals(1),  0) << "depth=1  에서 체인 순회 없어야 한다";
    EXPECT_EQ(countTraversals(13), 0) << "depth=13 에서 체인 순회 없어야 한다";
    EXPECT_EQ(countTraversals(50), 0) << "depth=50 에서 체인 순회 없어야 한다";
}

// ── TC4: 클로저 — 비전역 변수 캡처 정확성 검증 ─────────────────────────
// makeAdder 패턴: 함수 반환 후 다른 scope stack에서 호출해도
// 올바른 클로저 환경(makeAdder의 x)에 접근해야 한다
TEST(StaticBindingTest, Closure_NonGlobalCapture_CorrectResult) {
    // LangFactory를 통해 실행 (setBindings 활성화 상태)
    const std::string source =
        "func makeAdder(x) {"
        "    func adder(y) { return x + y; }"
        "    return adder;"
        "}"
        "var add5 = makeAdder(5);"
        "print add5(3);";   // 기대값: 8

    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    LangFactory factory;
    factory.run(source);
    std::cout.rdbuf(old);

    EXPECT_EQ(oss.str(), "8\n")
        << "클로저가 올바른 closure env를 참조해야 한다 (scope stack 재구성 검증)";
}

// ── TC5: for 루프 + 중첩 블록 — 요구사항 예시 코드 직접 검증 ────────────
// 요구사항의 for (var i ...) { a = a + 1; } 패턴을 실행하여
// 정적 바인딩 하에서 체인 순회 없이 정확한 결과가 나와야 한다
TEST(StaticBindingTest, ForLoop_NestedBlock_UsesDirectAccess) {
    const std::string source = R"(
{
    var a = 0;
    { { { { { { { { { { { { {
    for (var i = 0; i < 10; i = i + 1) {
        a = a + 1;
    }
    } } } } } } } } } } } } }
    print a;
}
)";
    auto [stmts, bindings] = parseAndResolve(source);

    Interpreter::BindingSpy spy;
    Interpreter interp;
    interp.setBindings(&bindings);
    interp.setSpy(&spy);

    Environment::resetChainSteps();
    Environment::resetGetAtHops();

    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    interp.interpret(stmts);
    std::cout.rdbuf(old);

    EXPECT_EQ(oss.str(), "10\n")         << "for 루프가 정확히 10회 실행돼야 한다";
    EXPECT_EQ(Environment::chainSteps(), 0) << "for 루프 내 지역 변수 접근에 체인 탐색이 없어야 한다";
    EXPECT_EQ(Environment::getAtHops(),  0) << "getAt 포인터 순회도 발생하지 않아야 한다";
}
