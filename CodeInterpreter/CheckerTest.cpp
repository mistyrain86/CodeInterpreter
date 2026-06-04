#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "Checker.h"
#include "LangFactory.h"
#include "TestUtils.h"

using ::testing::_;

TEST(CheckerUnit, EmptyProgram_NoThrow) {
    EXPECT_NO_THROW(Checker().check({}));
}
TEST(CheckerUnit, GlobalVarDecl_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(10.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}