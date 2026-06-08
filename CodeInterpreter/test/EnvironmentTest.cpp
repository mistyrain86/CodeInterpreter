#include <gtest/gtest.h>
#include "Environment.h"

static Token tok(std::string name, int line = 1) {
    return Token{TokenType::IDENTIFIER, std::move(name), std::monostate{}, line};
}

class EnvironmentFixture : public ::testing::Test {
protected:
    Environment env;
};

TEST_F(EnvironmentFixture, DefineAndGet) {
    env.define("a", Value{10.0});
    EXPECT_DOUBLE_EQ(std::get<double>(env.get(tok("a"))), 10.0);
}
TEST_F(EnvironmentFixture, DefineString) {
    env.define("s", Value{std::string("hi")});
    EXPECT_EQ(std::get<std::string>(env.get(tok("s"))), "hi");
}
TEST_F(EnvironmentFixture, UndefinedVariable_Throws) {
    EXPECT_THROW(env.get(tok("x", 5)), std::runtime_error);
}
TEST_F(EnvironmentFixture, ErrorMsg_HasLineAndName) {
    try { env.get(tok("missing", 7)); FAIL(); }
    catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("7"),       std::string::npos);
        EXPECT_NE(msg.find("missing"), std::string::npos);
    }
}
TEST_F(EnvironmentFixture, Assign_UpdatesValue) {
    env.define("a", Value{1.0});
    env.assign(tok("a"), Value{99.0});
    EXPECT_DOUBLE_EQ(std::get<double>(env.get(tok("a"))), 99.0);
}
TEST_F(EnvironmentFixture, Assign_Undefined_Throws) {
    EXPECT_THROW(env.assign(tok("x"), Value{1.0}), std::runtime_error);
}

// TASK-D02: 스코프 체인 테스트
class ScopeFixture : public ::testing::Test {
protected:
    std::shared_ptr<Environment> global = std::make_shared<Environment>();
    Environment                  local{global};
};

TEST_F(ScopeFixture, LookupInEnclosing) {
    global->define("x", Value{42.0});
    EXPECT_DOUBLE_EQ(std::get<double>(local.get(tok("x"))), 42.0);
}
TEST_F(ScopeFixture, Shadowing_LocalFirst) {
    global->define("x", Value{1.0});
    local.define("x", Value{2.0});
    EXPECT_DOUBLE_EQ(std::get<double>(local.get(tok("x"))), 2.0);
    EXPECT_DOUBLE_EQ(std::get<double>(global->get(tok("x"))), 1.0);
}
TEST_F(ScopeFixture, AssignInEnclosing_UpdatesOuter) {
    global->define("count", Value{0.0});
    local.assign(tok("count"), Value{1.0});
    EXPECT_DOUBLE_EQ(std::get<double>(global->get(tok("count"))), 1.0);
}
TEST_F(ScopeFixture, ThreeLevels_DeepLookup) {
    auto level1 = std::make_shared<Environment>();
    level1->define("a", Value{10.0});
    auto level2 = std::make_shared<Environment>(level1);
    auto level3 = std::make_shared<Environment>(level2);
    EXPECT_DOUBLE_EQ(std::get<double>(level3->get(tok("a"))), 10.0);
}

// 커버리지 보강
TEST_F(ScopeFixture, Assign_ThreeLevels_UpdatesRoot) {
    auto level1 = std::make_shared<Environment>();
    level1->define("a", Value{1.0});
    auto level2 = std::make_shared<Environment>(level1);
    auto level3 = std::make_shared<Environment>(level2);
    level3->assign(tok("a"), Value{99.0});
    EXPECT_DOUBLE_EQ(std::get<double>(level1->get(tok("a"))), 99.0);
}
TEST_F(ScopeFixture, Assign_Shadowed_UpdatesLocal) {
    global->define("x", Value{1.0});
    local.define("x", Value{2.0});
    local.assign(tok("x"), Value{99.0});
    EXPECT_DOUBLE_EQ(std::get<double>(local.get(tok("x"))),   99.0);
    EXPECT_DOUBLE_EQ(std::get<double>(global->get(tok("x"))),  1.0);
}
TEST_F(EnvironmentFixture, Define_Overwrite_SameName) {
    env.define("x", Value{1.0});
    env.define("x", Value{42.0});
    EXPECT_DOUBLE_EQ(std::get<double>(env.get(tok("x"))), 42.0);
}

// 커버리지 보강: get/assign의 enclosing 체인 탐색 후 throw 경로
TEST_F(ScopeFixture, Get_NotInAnyScope_Throws) {
    auto level1 = std::make_shared<Environment>();
    auto level2 = std::make_shared<Environment>(level1);
    EXPECT_THROW(level2->get(tok("notDefined")), std::runtime_error);
}

TEST_F(ScopeFixture, Assign_NotInAnyScope_Throws) {
    auto level1 = std::make_shared<Environment>();
    auto level2 = std::make_shared<Environment>(level1);
    EXPECT_THROW(level2->assign(tok("notDefined"), Value{1.0}), std::runtime_error);
}
