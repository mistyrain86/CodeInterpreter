#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "LangFactory.h"
#include "TestUtils.h"

using ::testing::_;
using ::testing::InSequence;

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
