#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "Parser.h"
#include "LangFactory.h"
#include "TestUtils.h"

using ::testing::_;
using ::testing::Return;

// ── 단위 테스트 헬퍼 ──────────────────────────────────────
static Token t(TokenType type, std::string lex,
               std::variant<std::monostate,double,std::string> lit = std::monostate{},
               int line = 1) {
    return Token{type, std::move(lex), std::move(lit), line};
}
static Token eof()  { return t(TokenType::END_OF_FILE, ""); }
static Token semi() { return t(TokenType::SEMICOLON, ";"); }

static std::vector<StmtPtr> parse(std::vector<Token> tokens) {
    return Parser().parse(std::move(tokens));
}

// ── 단위 테스트 ───────────────────────────────────────────
TEST(ParserUnit, NumberLiteral) {
    auto stmts = parse({t(TokenType::NUMBER,"5",5.0), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 5.0);
}
TEST(ParserUnit, StringLiteral) {
    auto stmts = parse({t(TokenType::STRING,"\"hi\"",std::string("hi")), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<std::string>(lit->value), "hi");
}
TEST(ParserUnit, BoolTrue) {
    auto stmts = parse({t(TokenType::KW_TRUE,"true"), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<bool>(lit->value), true);
}
TEST(ParserUnit, BoolFalse) {
    auto stmts = parse({t(TokenType::KW_FALSE,"false"), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<bool>(lit->value), false);
}
TEST(ParserUnit, Grouping) {
    auto stmts = parse({t(TokenType::LEFT_PAREN,"("),
                        t(TokenType::NUMBER,"5",5.0),
                        t(TokenType::RIGHT_PAREN,")"), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<GroupingExpr*>(es->expression.get()), nullptr);
}

// ── Mock 통합 테스트 ─────────────────────────────────────
TEST(ParserMock, NumberLiteral_PassesThrough) {
    auto ml = std::make_unique<MockLexer>();
    EXPECT_CALL(*ml, tokenize(_)).WillOnce(Return(std::vector<Token>{
        t(TokenType::NUMBER,"5",5.0), semi(), eof()
    }));
    auto mc = std::make_unique<MockChecker>();
    EXPECT_CALL(*mc, check(_)).Times(1);
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    LangFactory factory(std::move(ml), std::make_unique<Parser>(),
                        std::move(mc), std::move(mi));
    EXPECT_NO_THROW(factory.run("5;"));
}
