#include <gtest/gtest.h>
#include "Lexer.h"
#include "Parser.h"
#include "Resolver.h"
#include "Interpreter.h"
#include "ConstantFolder.h"

// 파싱 + 폴딩(옵션) + 리졸브
static std::pair<std::vector<StmtPtr>, BindingMap>
parseOptAndResolve(const std::string& source, bool fold) {
    Lexer  lexer;
    Parser parser;
    auto tokens = lexer.tokenize(source);
    auto stmts  = parser.parse(std::move(tokens));
    if (fold) {
        ConstantFolder folder;
        stmts = folder.optimize(std::move(stmts));
    }
    Resolver resolver;
    auto bindings = resolver.resolve(stmts);
    return { std::move(stmts), std::move(bindings) };
}

// 실행하고 binary op 횟수 반환
static int countBinaryOps(const std::string& source, bool fold) {
    auto [stmts, bindings] = parseOptAndResolve(source, fold);
    Interpreter::OpSpy opSpy;
    Interpreter interp;
    interp.setBindings(&bindings);
    interp.setOpSpy(&opSpy);
    interp.interpret(stmts);
    return opSpy.m_binaryOpCount;
}

// ── TC1: AST 수준 검증 ────────────────────────────────────────────────
// ConstantFolder가 상수 표현식을 단일 LiteralExpr로 교체했는지 확인
TEST(ConstantFolderTest, FoldsConstExpr_ToLiteral) {
    // 상수 표현식: (1 - 2 * 3 * 4 * 5 / 6 + 7 + 8 + 9) % 1000 % 30 = 5
    const std::string source = R"(
var total = 0;
for (var i = 0; i < 1; i = i + 1) {
    total = total + (1 - 2 * 3 * 4 * 5 / 6 + 7 + 8 + 9) % 1000 % 30;
}
)";
    Lexer lexer; Parser parser;
    auto stmts = parser.parse(lexer.tokenize(source));

    ConstantFolder folder;
    stmts = folder.optimize(std::move(stmts));

    // stmts[1] = ForStmt
    // ForStmt.m_body = BlockStmt
    // BlockStmt.m_statements[0] = ExprStmt (total = total + ...)
    // ExprStmt.m_expression = AssignExpr
    // AssignExpr.value = BinaryExpr(total + CONST)
    // BinaryExpr.right = LiteralExpr(5) ← 검증 대상

    auto* forStmt   = dynamic_cast<ForStmt*>(stmts[1].get());
    auto* blockStmt = dynamic_cast<BlockStmt*>(forStmt->m_body.get());
    auto* exprStmt  = dynamic_cast<ExprStmt*>(blockStmt->m_statements[0].get());
    auto* assignExpr = dynamic_cast<AssignExpr*>(exprStmt->m_expression.get());
    auto* addExpr   = dynamic_cast<BinaryExpr*>(assignExpr->value.get());
    auto* folded    = dynamic_cast<LiteralExpr*>(addExpr->right.get());

    ASSERT_NE(folded, nullptr)
        << "상수 표현식이 LiteralExpr로 교체돼야 한다";
    EXPECT_EQ(std::get<double>(folded->value), 5.0)
        << "폴딩 결과가 5.0이어야 한다";
}

// ── TC2: 이진 연산 횟수 검증 ──────────────────────────────────────────
// 상수 폴딩 전: 루프 N회 동안 상수 표현식의 10개 연산이 N번 반복
// 상수 폴딩 후: 상수 표현식 연산이 0회 (리터럴로 대체됨)
TEST(ConstantFolderTest, ConstExpr_BinaryOpsReduced) {
    // 상수 표현식의 이진 연산 수: 10개
    // (2*3), (*4), (*5), (/6), (1-), (+7), (+8), (+9), (%1000), (%30)
    constexpr int LOOP_COUNT         = 5;
    constexpr int CONST_OPS_PER_ITER = 10;  // 상수 표현식 내 이진 연산 수

    const std::string source = R"(
var total = 0;
for (var i = 0; i < 5; i = i + 1) {
    total = total + (1 - 2 * 3 * 4 * 5 / 6 + 7 + 8 + 9) % 1000 % 30;
}
)";

    int withoutFold = countBinaryOps(source, false);
    int withFold    = countBinaryOps(source, true);

    // 핵심 검증: 폴딩으로 줄어든 연산 수 = LOOP_COUNT * CONST_OPS_PER_ITER
    EXPECT_EQ(withoutFold - withFold, LOOP_COUNT * CONST_OPS_PER_ITER)
        << "상수 표현식 " << CONST_OPS_PER_ITER << "개 연산이 "
        << LOOP_COUNT << "회 반복 → " << LOOP_COUNT * CONST_OPS_PER_ITER
        << "회 감소해야 한다";

    // 폴딩 후 상수 연산 횟수 = 0 (런타임에 계산 없음)
    // (루프 오버헤드: i<5 = 6회, i+1 = 5회, total+5 = 5회 = 16회만 남음)
    EXPECT_EQ(withFold, withoutFold - LOOP_COUNT * CONST_OPS_PER_ITER)
        << "폴딩 후 실행 시 상수 연산이 완전히 제거돼야 한다";
}

// ── TC3: 결과 정확성 검증 ─────────────────────────────────────────────
// 폴딩 여부와 관계없이 동일한 결과를 내야 한다
TEST(ConstantFolderTest, ConstExpr_CorrectResult) {
    // 5회 반복 × 5 = 25
    const std::string source = R"(
var total = 0;
for (var i = 0; i < 5; i = i + 1) {
    total = total + (1 - 2 * 3 * 4 * 5 / 6 + 7 + 8 + 9) % 1000 % 30;
}
print total;
)";

    auto runAndCapture = [&](bool fold) -> std::string {
        auto [stmts, bindings] = parseOptAndResolve(source, fold);
        std::ostringstream oss;
        auto* old = std::cout.rdbuf(oss.rdbuf());
        Interpreter interp;
        interp.setBindings(&bindings);
        interp.interpret(stmts);
        std::cout.rdbuf(old);
        return oss.str();
    };

    EXPECT_EQ(runAndCapture(false), "25\n") << "폴딩 없이 실행 결과가 25이어야 한다";
    EXPECT_EQ(runAndCapture(true),  "25\n") << "폴딩 후 실행 결과가 25이어야 한다";
}
