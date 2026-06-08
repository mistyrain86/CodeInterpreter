#include <gtest/gtest.h>
#include "ConstantFolder.h"
#include "Interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "Resolver.h"
#include "TestUtils.h"

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
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        binaryExpr(binaryExpr(litNum(3), TokenType::PLUS, "+", litNum(4)),
            TokenType::STAR, "*", litNum(2))));
    auto folded = m_folder.optimize(std::move(stmts));
    EXPECT_EQ(runAll(std::move(folded)), "14\n");
}

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

static int countBinaryOps(const std::string& source, bool fold) {
    auto [stmts, bindings] = parseOptAndResolve(source, fold);
    Interpreter::OpSpy opSpy;
    Interpreter interp;
    interp.setBindings(&bindings);
    interp.setOpSpy(&opSpy);
    interp.interpret(stmts);
    return opSpy.m_binaryOpCount;
}

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

TEST_F(ConstantFolderFixture, Fold_Percent) {
    EXPECT_DOUBLE_EQ(foldToDouble(binaryExpr(litNum(10), TokenType::PERCENT, "%", litNum(3))), 1.0);
}

TEST_F(ConstantFolderFixture, NoFold_PercentByZero) {
    EXPECT_FALSE(wasFolded(binaryExpr(litNum(10), TokenType::PERCENT, "%", litNum(0))));
}

TEST_F(ConstantFolderFixture, NoFold_GroupingWithVariable) {
    auto grouping = std::make_unique<GroupingExpr>(
        binaryExpr(varRef("x"), TokenType::PLUS, "+", litNum(1.0)));
    EXPECT_FALSE(wasFolded(std::move(grouping)));
}

TEST(ConstantFolderTest, VisitIfStmt_FoldsCondition) {
    ConstantFolder folder;
    std::vector<StmtPtr> body;
    body.push_back(printStmt(binaryExpr(litNum(2), TokenType::STAR, "*", litNum(3))));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<IfStmt>(0,
        binaryExpr(litNum(1), TokenType::PLUS, "+", litNum(1)),
        blockStmt(std::move(body)),
        nullptr));
    auto result = folder.optimize(std::move(stmts));
    auto* ifs  = dynamic_cast<IfStmt*>(result[0].get());
    ASSERT_NE(ifs, nullptr);
    auto* cond = dynamic_cast<LiteralExpr*>(ifs->m_condition.get());
    ASSERT_NE(cond, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(cond->value), 2.0);
}

TEST(ConstantFolderTest, VisitFunctionStmt_FoldsReturnBody) {
    ConstantFolder folder;
    Token retTok = Token{TokenType::KW_RETURN, "return", std::monostate{}, 1};
    std::vector<StmtPtr> body;
    body.push_back(std::make_unique<ReturnStmt>(
        retTok, binaryExpr(litNum(2), TokenType::PLUS, "+", litNum(3))));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<FunctionStmt>(
        makeIdent("f"), std::vector<Token>{}, std::move(body)));
    auto result = folder.optimize(std::move(stmts));
    auto* fn  = dynamic_cast<FunctionStmt*>(result[0].get());
    ASSERT_NE(fn, nullptr);
    auto* ret = dynamic_cast<ReturnStmt*>(fn->m_body[0].get());
    ASSERT_NE(ret, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(ret->m_value.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 5.0);
}
