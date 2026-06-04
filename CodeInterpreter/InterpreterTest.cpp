#include <gtest/gtest.h>
#include "Interpreter.h"
#include "TestUtils.h"

static Token opTok(TokenType t, std::string lex, int line = 1) {
    return Token{t, std::move(lex), std::monostate{}, line};
}
static std::string run(std::vector<StmtPtr> stmts) {
    return captureOutput([&]{ Interpreter().interpret(stmts); });
}

TEST(InterpreterTest, PrintInteger)   { std::vector<StmtPtr> s; s.push_back(printStmt(litNum(5.0)));
EXPECT_EQ(run(std::move(s)), "5\n");     }
TEST(InterpreterTest, PrintFloat)     { std::vector<StmtPtr> s; s.push_back(printStmt(litNum(3.14)));
EXPECT_EQ(run(std::move(s)), "3.14\n");  }
TEST(InterpreterTest, PrintIntFormat) { std::vector<StmtPtr> s; s.push_back(printStmt(litNum(5.0)));
EXPECT_EQ(run(std::move(s)), "5\n");     }
TEST(InterpreterTest, PrintString)    { std::vector<StmtPtr> s; s.push_back(printStmt(litStr("hello")));
EXPECT_EQ(run(std::move(s)), "hello\n"); }
TEST(InterpreterTest, PrintBoolTrue)  { std::vector<StmtPtr> s; s.push_back(printStmt(litBool(true)));
EXPECT_EQ(run(std::move(s)), "true\n");  }
TEST(InterpreterTest, PrintBoolFalse) { std::vector<StmtPtr> s; s.push_back(printStmt(litBool(false)));
EXPECT_EQ(run(std::move(s)), "false\n"); }

TEST(InterpreterTest, UnaryMinus) {
    std::vector<StmtPtr> s;
    s.push_back(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::MINUS, "-"), litNum(3.0))));
    EXPECT_EQ(run(std::move(s)), "-3\n");
}
TEST(InterpreterTest, UnaryBang_True) {
    std::vector<StmtPtr> s;
    s.push_back(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::BANG, "!"), litBool(true))));
    EXPECT_EQ(run(std::move(s)), "false\n");
}
TEST(InterpreterTest, UnaryMinus_OnString_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ExprStmt>(
        std::make_unique<UnaryExpr>(opTok(TokenType::MINUS, "-"), litStr("oops"))));
    Interpreter interp;
    EXPECT_THROW(interp.interpret(s), RuntimeError);
}
