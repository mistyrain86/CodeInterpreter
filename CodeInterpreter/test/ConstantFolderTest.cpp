#include <gtest/gtest.h>
#include "ConstantFolder.h"
#include "Interpreter.h"
#include "TestUtils.h"

static ExprPtr bin(ExprPtr l, TokenType op, std::string lex, ExprPtr r) {
    return std::make_unique<BinaryExpr>(
        std::move(l), Token{op, std::move(lex), std::monostate{}, 1}, std::move(r));
}

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
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(3),  TokenType::PLUS,  "+", litNum(4))),  7.0);
}
TEST_F(ConstantFolderFixture, Fold_Minus) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(10), TokenType::MINUS, "-", litNum(3))),  7.0);
}
TEST_F(ConstantFolderFixture, Fold_Star) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(3),  TokenType::STAR,  "*", litNum(4))), 12.0);
}
TEST_F(ConstantFolderFixture, Fold_Slash) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(8),  TokenType::SLASH, "/", litNum(2))),  4.0);
}
TEST_F(ConstantFolderFixture, NoFold_DivisionByZero) {
    EXPECT_FALSE(wasFolded(bin(litNum(1), TokenType::SLASH, "/", litNum(0))));
}
TEST_F(ConstantFolderFixture, NoFold_WithVariable) {
    EXPECT_FALSE(wasFolded(bin(varRef("x"), TokenType::PLUS, "+", litNum(1))));
}
TEST_F(ConstantFolderFixture, Fold_Nested) {
    auto inner = bin(litNum(1), TokenType::PLUS, "+", litNum(2));
    auto outer = bin(std::move(inner), TokenType::STAR, "*", litNum(3));
    EXPECT_DOUBLE_EQ(foldToDouble(std::move(outer)), 9.0);
}

TEST(ConstantFolderTest, Fold_VarStmt_Initializer) {
    ConstantFolder folder;
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", bin(litNum(2), TokenType::STAR, "*", litNum(3))));
    auto result = folder.optimize(std::move(stmts));
    auto* vs  = dynamic_cast<VarStmt*>(result[0].get());
    auto* lit = dynamic_cast<LiteralExpr*>(vs->m_initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 6.0);
}

TEST_F(ConstantFolderFixture, RunResult) {
    // (3 + 4) * 2 → 폴딩 후 LiteralExpr(14) → 실행 결과 14
    auto expr = bin(
        bin(litNum(3), TokenType::PLUS, "+", litNum(4)),
        TokenType::STAR, "*", litNum(2));
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(std::move(expr)));
    auto folded = m_folder.optimize(std::move(stmts));
    EXPECT_EQ(runAll(std::move(folded)), "14\n");
}
