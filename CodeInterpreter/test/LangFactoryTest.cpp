#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "LangFactory.h"
#include "ParseError.h"
#include "CheckError.h"
#include "Parser.h"
#include "Checker.h"
#include "Lexer.h"
#include "RuntimeError.h"
#include "ConstantFolder.h"
#include "TestUtils.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Throw;

static auto emptyParse() {
    return ::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> { return {}; });
}

class LangFactoryMockFixture : public ::testing::Test {
protected:
    MockLexer*       ml = nullptr;
    MockParser*      mp = nullptr;
    MockChecker*     mc = nullptr;
    MockInterpreter* mi = nullptr;
    std::unique_ptr<LangFactory> factory;

    void SetUp() override {
        auto lexer       = std::make_unique<MockLexer>();
        auto parser      = std::make_unique<MockParser>();
        auto checker     = std::make_unique<MockChecker>();
        auto interpreter = std::make_unique<MockInterpreter>();

        ml = lexer.get();
        mp = parser.get();
        mc = checker.get();
        mi = interpreter.get();

        factory = std::make_unique<LangFactory>(
            std::move(lexer), std::move(parser),
            std::move(checker), std::move(interpreter));
    }
};

TEST_F(LangFactoryMockFixture, Pipeline_CallsInOrder) {
    InSequence seq;
    EXPECT_CALL(*ml, tokenize("print 5;")).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(emptyParse());
    EXPECT_CALL(*mc, check(_)).Times(1);
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    factory->run("print 5;");
}

TEST_F(LangFactoryMockFixture, ParseError_StopsBeforeChecker) {
    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(Throw(ParseError("[라인 1] 구문 오류: 테스트")));
    EXPECT_CALL(*mc, check(_)).Times(0);
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), ParseError);
}

TEST_F(LangFactoryMockFixture, CheckerError_StopsBeforeInterpreter) {
    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(emptyParse());
    EXPECT_CALL(*mc, check(_))
        .WillOnce(Throw(CheckError("[라인 1] 의미 오류: 테스트")));
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), CheckError);
}

// ── Real Lexer + Real Parser + Real Checker 통합 픽스처 ─────────
class LangFactoryIntegrationFixture : public ::testing::Test {
protected:
    MockInterpreter* mi = nullptr;
    std::unique_ptr<LangFactory> factory;

    void SetUp() override {
        auto interpreter = std::make_unique<MockInterpreter>();
        mi = interpreter.get();

        factory = std::make_unique<LangFactory>(
            std::make_unique<Lexer>(),
            std::make_unique<Parser>(),
            std::make_unique<Checker>(),
            std::move(interpreter));
    }
};

TEST_F(LangFactoryIntegrationFixture, PrintStmt_Integration) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    factory->run("print 5;");
}

TEST_F(LangFactoryIntegrationFixture, VarDecl_Integration) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    factory->run("var x = 10;");
}

TEST_F(LangFactoryIntegrationFixture, PrintLiteral_CheckerPasses) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    EXPECT_NO_THROW(factory->run("print 42;"));
}

TEST_F(LangFactoryIntegrationFixture, VarDeclAndRef_CheckerPasses) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    EXPECT_NO_THROW(factory->run("var x = 10; print x;"));
}

TEST_F(LangFactoryIntegrationFixture, ParseError_IncompleteSyntax) {
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    EXPECT_THROW(factory->run("var"), ParseError);
}

TEST_F(LangFactoryIntegrationFixture, DuplicateVar_InBlock_Throws) {
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    EXPECT_THROW(factory->run("{ var x = 1; var x = 2; }"), CheckError);
}

TEST_F(LangFactoryIntegrationFixture, SelfReference_InBlock_Throws) {
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    EXPECT_THROW(factory->run("{ var x = x; }"), CheckError);
}

TEST_F(LangFactoryMockFixture, LexerError_StopsBeforeParser) {
    EXPECT_CALL(*ml, tokenize(_))
        .WillOnce(::testing::Throw(std::runtime_error("lexer error")));
    EXPECT_CALL(*mp, parse(_)).Times(0);
    EXPECT_CALL(*mc, check(_)).Times(0);
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), std::runtime_error);
}

TEST_F(LangFactoryIntegrationFixture, RuntimeError_Propagates) {
    EXPECT_CALL(*mi, interpret(_))
        .WillOnce([](const std::vector<StmtPtr>&) {
            throw RuntimeError("0으로 나눌 수 없습니다.");
        });
    EXPECT_THROW(factory->run("print 1 / 0;"), RuntimeError);
}

TEST_F(LangFactoryIntegrationFixture, Complex_VarAndArith_Integration) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    EXPECT_NO_THROW(factory->run("var a = 1; var b = 2; print a + b;"));
}

// Lexer 특화 테스트
TEST_F(LangFactoryIntegrationFixture, StringLiteral_EndToEnd) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    EXPECT_NO_THROW(factory->run("print \"hello\";"));
}

TEST_F(LangFactoryIntegrationFixture, LineComment_Ignored) {
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    EXPECT_NO_THROW(factory->run("// 주석\nprint 5;"));
}

TEST_F(LangFactoryIntegrationFixture, UnexpectedChar_LexerError) {
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    EXPECT_THROW(factory->run("@"), std::runtime_error);
}

TEST(LangFactoryFull, SetOptimizer_RunsOptimizer) {
    LangFactory factory;
    factory.setOptimizer(std::make_unique<ConstantFolder>());
    EXPECT_EQ(captureOutput([&]{ factory.run("print 2 + 3;"); }), "5\n");
}

TEST(LangFactoryFull, SetOptimizer_ConstantFolding_ReducesBinaryOps) {
    LangFactory factory;
    factory.setOptimizer(std::make_unique<ConstantFolder>());
    EXPECT_EQ(captureOutput([&]{ factory.run("print 10 * 10 + 5;"); }), "105\n");
}
