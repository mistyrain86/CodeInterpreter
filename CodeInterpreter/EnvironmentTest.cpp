#include <gtest/gtest.h>
#include "Environment.h"
#include "TestUtils.h"

static Token tok(std::string name, int line = 1) {
    return Token{TokenType::IDENTIFIER, std::move(name), std::monostate{}, line};
}

TEST(EnvironmentTest, DefineAndGet) {
    Environment env;
    env.define("a", Value{10.0});
    EXPECT_DOUBLE_EQ(std::get<double>(env.get(tok("a"))), 10.0);
}
TEST(EnvironmentTest, DefineString) {
    Environment env;
    env.define("s", Value{std::string("hi")});
    EXPECT_EQ(std::get<std::string>(env.get(tok("s"))), "hi");
}
TEST(EnvironmentTest, UndefinedVariable_Throws) {
    Environment env;
    EXPECT_THROW(env.get(tok("x", 5)), std::runtime_error);
}
TEST(EnvironmentTest, ErrorMsg_HasLineAndName) {
    Environment env;
    try { env.get(tok("missing", 7)); FAIL(); }
    catch (const std::runtime_error& e) {
        std::string msg = e.what();
        EXPECT_NE(msg.find("7"),       std::string::npos);
        EXPECT_NE(msg.find("missing"), std::string::npos);
    }
}
TEST(EnvironmentTest, Assign_UpdatesValue) {
    Environment env;
    env.define("a", Value{1.0});
    env.assign(tok("a"), Value{99.0});
    EXPECT_DOUBLE_EQ(std::get<double>(env.get(tok("a"))), 99.0);
}
TEST(EnvironmentTest, Assign_Undefined_Throws) {
    Environment env;
    EXPECT_THROW(env.assign(tok("x"), Value{1.0}), std::runtime_error);
}
