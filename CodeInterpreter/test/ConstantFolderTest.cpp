#include <gtest/gtest.h>
#include "ConstantFolder.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "Resolver.h"
#include "TestUtils.h"

// ── Fixture 기반 단위 TC (upstream) ──────────────────────────────────


class ConstantFolderFixture : public ::testing::Test {
protected:
    ConstantFolder       m_folder;
    Interpreter          m_interp;
    std::vector<StmtPtr> m_result;

    double foldToDouble(ExprPtr expr) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(printStmt(std::move(expr)));
        m_result = m_folder.optimize(std::move(stmts));
        auto* ps  = dynamic_cast<PrintStmt*>(m_result[0].get());
        auto* lit = dynamic_cast<LiteralExpr*>(ps->m_expression.get());
        EXPECT_NE(lit, nullptr) << "표현식이 LiteralExpr로 폴딩되지 않았습니다";
        if (!lit) return 0.0;
        return std::get<double>(lit->value);
    }

    bool wasFolded(ExprPtr expr) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(printStmt(std::move(expr)));
        m_result = m_folder.optimize(std::move(stmts));
        auto* ps = dynamic_cast<PrintStmt*>(m_result[0].get());
        return dynamic_cast<LiteralExpr*>(ps->m_expression.get()) != nullptr;
    }

    std::string runAll(std::vector<StmtPtr> stmts) {
        return captureOutput([&]{ m_interp.interpret(stmts); });
    }
};

TEST_F(ConstantFolderFixture, Fold_Plus) {
    EXPECT_DOUBLE_EQ(foldToDouble(binaryExpr(litNum(3),  TokenType::PLUS,  "+", litNum(4))),  7.0);
}
TEST_F(ConstantFolderFixture, Fold_Minus) {
    EXPECT_DOUBLE_EQ(foldToDouble(binaryExpr(litNum(10), TokenType::MINUS, "-", litNum(3))),  7.0);
}
TEST_F(ConstantFolderFixture, Fold_Star) {
    EXPECT_DOUBLE_EQ(foldToDouble(binaryExpr(litNum(3),  TokenType::STAR,  "*", litNum(4))), 12.0);
}
TEST_F(ConstantFolderFixture, Fold_Slash) {
    EXPECT_DOUBLE_EQ(foldToDouble(binaryExpr(litNum(8),  TokenType::SLASH, "/", litNum(2))),  4.0);
}
TEST_F(ConstantFolderFixture, NoFold_DivisionByZero) {
    EXPECT_FALSE(wasFolded(binaryExpr(litNum(1), TokenType::SLASH, "/", litNum(0))));
}
TEST_F(ConstantFolderFixture, NoFold_WithVariable) {
    EXPECT_FALSE(wasFolded(binaryExpr(varRef("x"), TokenType::PLUS, "+", litNum(1))));
}
TEST_F(ConstantFolderFixture, Fold_Nested) {
    auto inner = binaryExpr(litNum(1), TokenType::PLUS, "+", litNum(2));
    auto outer = binaryExpr(std::move(inner), TokenType::STAR, "*", litNum(3));
    EXPECT_DOUBLE_EQ(foldToDouble(std::move(outer)), 9.0);
}

TEST(ConstantFolderTest, Fold_VarStmt_Initializer) {
    ConstantFolder folder;
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", binaryExpr(litNum(2), TokenType::STAR, "*", litNum(3))));
    auto result = folder.optimize(std::move(stmts));
    auto* vs  = dynamic_cast<VarStmt*>(result[0].get());
    auto* lit = dynamic_cast<LiteralExpr*>(vs->m_initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 6.0);
}

TEST_F(ConstantFolderFixture, RunResult) {
    // (3 + 4) * 2 → 폴딩 후 LiteralExpr(14) → 실행 결과 14
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        binaryExpr(binaryExpr(litNum(3), TokenType::PLUS, "+", litNum(4)),
            TokenType::STAR, "*", litNum(2))));
    auto folded = m_folder.optimize(std::move(stmts));
    EXPECT_EQ(runAll(std::move(folded)), "14\n");
}

// ── Test Double 기반 TC (상수 합치기 최적화 검증) ────────────────────

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

// TC: AST 수준 검증
// ConstantFolder가 상수 표현식을 단일 LiteralExpr로 교체했는지 확인
TEST(ConstantFolderTest, FoldsConstExpr_ToLiteral) {
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

    auto* forStmt    = dynamic_cast<ForStmt*>(stmts[1].get());
    auto* blockStmt  = dynamic_cast<BlockStmt*>(forStmt->m_body.get());
    auto* exprStmt   = dynamic_cast<ExprStmt*>(blockStmt->m_statements[0].get());
    auto* assignExpr = dynamic_cast<AssignExpr*>(exprStmt->m_expression.get());
    auto* addExpr    = dynamic_cast<BinaryExpr*>(assignExpr->value.get());
    auto* folded     = dynamic_cast<LiteralExpr*>(addExpr->right.get());

    ASSERT_NE(folded, nullptr) << "상수 표현식이 LiteralExpr로 교체돼야 한다";
    EXPECT_EQ(std::get<double>(folded->value), 5.0) << "폴딩 결과가 5.0이어야 한다";
}

// TC: 이진 연산 횟수 검증
// 상수 폴딩 전: 루프 N회 동안 상수 표현식의 10개 연산이 N번 반복
// 상수 폴딩 후: 상수 표현식 연산이 0회 (리터럴로 대체됨)
TEST(ConstantFolderTest, ConstExpr_BinaryOpsReduced) {
    constexpr int LOOP_COUNT         = 5;
    constexpr int CONST_OPS_PER_ITER = 10;

    const std::string source = R"(
var total = 0;
for (var i = 0; i < 5; i = i + 1) {
    total = total + (1 - 2 * 3 * 4 * 5 / 6 + 7 + 8 + 9) % 1000 % 30;
}
)";

    int withoutFold = countBinaryOps(source, false);
    int withFold    = countBinaryOps(source, true);

    EXPECT_EQ(withoutFold - withFold, LOOP_COUNT * CONST_OPS_PER_ITER)
        << "상수 표현식 " << CONST_OPS_PER_ITER << "개 연산이 "
        << LOOP_COUNT << "회 반복 → " << LOOP_COUNT * CONST_OPS_PER_ITER << "회 감소해야 한다";
}

// TC: 결과 정확성 검증 — 폴딩 여부와 관계없이 동일한 결과를 내야 한다
TEST(ConstantFolderTest, ConstExpr_CorrectResult) {
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
