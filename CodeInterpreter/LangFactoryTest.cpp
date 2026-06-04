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

TEST(LangFactoryTest, Pipeline_CallsInOrder) {
    InSequence seq;
    auto ml = std::make_unique<MockLexer>();
    auto mp = std::make_unique<MockParser>();
    auto mc = std::make_unique<MockChecker>();
    auto mi = std::make_unique<MockInterpreter>();

    EXPECT_CALL(*ml, tokenize("print 5;")).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs(
            []() -> std::vector<StmtPtr> { return {}; }));
    EXPECT_CALL(*mc, check(_)).Times(1);
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    LangFactory factory(std::move(ml), std::move(mp),
                        std::move(mc), std::move(mi));
    factory.run("print 5;");
}

TEST(LangFactoryTest, ParseError_StopsBeforeChecker) {
    auto ml = std::make_unique<MockLexer>();
    auto mp = std::make_unique<MockParser>();
    auto mc = std::make_unique<MockChecker>();
    auto mi = std::make_unique<MockInterpreter>();

    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(Throw(ParseError("[라인 1] 구문 오류: 테스트")));
    EXPECT_CALL(*mc, check(_)).Times(0);
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    LangFactory factory(std::move(ml), std::move(mp),
                        std::move(mc), std::move(mi));
    EXPECT_THROW(factory.run(""), ParseError);
}

TEST(LangFactoryTest, CheckerError_StopsBeforeInterpreter) {
    auto ml = std::make_unique<MockLexer>();
    auto mp = std::make_unique<MockParser>();
    auto mc = std::make_unique<MockChecker>();
    auto mi = std::make_unique<MockInterpreter>();

    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs(
            []() -> std::vector<StmtPtr> { return {}; }));
    EXPECT_CALL(*mc, check(_))
        .WillOnce(Throw(CheckError("[라인 1] 의미 오류: 테스트")));
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    LangFactory factory(std::move(ml), std::move(mp),
                        std::move(mc), std::move(mi));
    EXPECT_THROW(factory.run(""), CheckError);
}
