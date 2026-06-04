#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "LangFactory.h"
#include "ParseError.h"
#include "CheckError.h"
#include "TestUtils.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Throw;

static auto emptyParse() {
    return ::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> { return {}; });
}

class LangFactoryFixture : public ::testing::Test {
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

TEST_F(LangFactoryFixture, Pipeline_CallsInOrder) {
    InSequence seq;
    EXPECT_CALL(*ml, tokenize("print 5;")).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(emptyParse());
    EXPECT_CALL(*mc, check(_)).Times(1);
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    factory->run("print 5;");
}

TEST_F(LangFactoryFixture, ParseError_StopsBeforeChecker) {
    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(Throw(ParseError("[라인 1] 구문 오류: 테스트")));
    EXPECT_CALL(*mc, check(_)).Times(0);
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), ParseError);
}

TEST_F(LangFactoryFixture, CheckerError_StopsBeforeInterpreter) {
    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(emptyParse());
    EXPECT_CALL(*mc, check(_))
        .WillOnce(Throw(CheckError("[라인 1] 의미 오류: 테스트")));
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), CheckError);
}
