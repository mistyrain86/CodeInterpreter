#include <gtest/gtest.h>
#include "Interpreter.h"
#include "TestUtils.h"

static Token opTok(TokenType t, std::string lex, int line = 1) {
    return Token{t, std::move(lex), std::monostate{}, line};
}
static ExprPtr bin(ExprPtr l, TokenType op, std::string lex, ExprPtr r) {
    return std::make_unique<BinaryExpr>(
        std::move(l), Token{op, std::move(lex), std::monostate{}, 1}, std::move(r));
}

class InterpreterFixture : public ::testing::Test {
protected:
    std::string run(StmtPtr stmt) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(std::move(stmt));
        return captureOutput([&]{ Interpreter().interpret(stmts); });
    }
};

TEST_F(InterpreterFixture, PrintInteger)   { EXPECT_EQ(run(printStmt(litNum(5.0))),       "5\n");     }
TEST_F(InterpreterFixture, PrintFloat)     { EXPECT_EQ(run(printStmt(litNum(3.14))),      "3.14\n");  }
TEST_F(InterpreterFixture, PrintIntFormat) { EXPECT_EQ(run(printStmt(litNum(5.0))),       "5\n");     }
TEST_F(InterpreterFixture, PrintString)    { EXPECT_EQ(run(printStmt(litStr("hello"))),   "hello\n"); }
TEST_F(InterpreterFixture, PrintBoolTrue)  { EXPECT_EQ(run(printStmt(litBool(true))),     "true\n");  }
TEST_F(InterpreterFixture, PrintBoolFalse) { EXPECT_EQ(run(printStmt(litBool(false))),    "false\n"); }

TEST_F(InterpreterFixture, UnaryMinus) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::MINUS, "-"), litNum(3.0)))),
        "-3\n");
}
TEST_F(InterpreterFixture, UnaryBang_True) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::BANG, "!"), litBool(true)))),
        "false\n");
}
TEST_F(InterpreterFixture, UnaryMinus_OnString_Throws) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<UnaryExpr>(opTok(TokenType::MINUS, "-"), litStr("oops"))));
    Interpreter interp;
    EXPECT_THROW(interp.interpret(stmts), RuntimeError);
}

// 이항 연산
TEST_F(InterpreterFixture, Add)   { EXPECT_EQ(run(printStmt(bin(litNum(3),  TokenType::PLUS,  "+", litNum(4)))),  "7\n");  }
TEST_F(InterpreterFixture, Sub)   { EXPECT_EQ(run(printStmt(bin(litNum(10), TokenType::MINUS, "-", litNum(3)))),  "7\n");  }
TEST_F(InterpreterFixture, Mul)   { EXPECT_EQ(run(printStmt(bin(litNum(3),  TokenType::STAR,  "*", litNum(4)))),  "12\n"); }
TEST_F(InterpreterFixture, Div)   { EXPECT_EQ(run(printStmt(bin(litNum(8),  TokenType::SLASH, "/", litNum(2)))),  "4\n");  }
TEST_F(InterpreterFixture, StrConcat) {
    EXPECT_EQ(run(printStmt(bin(litStr("Hi"), TokenType::PLUS, "+", litStr("!")))), "Hi!\n");
}
TEST_F(InterpreterFixture, CmpLess_True) {
    EXPECT_EQ(run(printStmt(bin(litNum(1), TokenType::LESS,    "<", litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, CmpGreater_False) {
    EXPECT_EQ(run(printStmt(bin(litNum(3), TokenType::GREATER, ">", litNum(5)))), "false\n");
}
TEST_F(InterpreterFixture, TypeMismatch_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ExprStmt>(bin(litNum(1), TokenType::PLUS, "+", litStr("HI"))));
    Interpreter i;
    EXPECT_THROW(i.interpret(s), RuntimeError);
}
TEST_F(InterpreterFixture, DivByZero_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ExprStmt>(bin(litNum(1), TokenType::SLASH, "/", litNum(0))));
    Interpreter i;
    EXPECT_THROW(i.interpret(s), RuntimeError);
}

// 변수 & 제어흐름
TEST(InterpreterTest, VarDeclAndUse) {
    Interpreter interp;
    std::vector<StmtPtr> s;
    s.push_back(varDecl("a", litNum(10.0)));
    s.push_back(printStmt(varRef("a")));
    EXPECT_EQ(captureOutput([&]{ interp.interpret(s); }), "10\n");
}
TEST(InterpreterTest, Reassignment) {
    Interpreter interp;
    Token a = makeIdent("a");
    std::vector<StmtPtr> s;
    s.push_back(varDecl("a", litNum(1.0)));
    s.push_back(std::make_unique<ExprStmt>(
        std::make_unique<AssignExpr>(a, litNum(2.0))));
    s.push_back(printStmt(varRef("a")));
    EXPECT_EQ(captureOutput([&]{ interp.interpret(s); }), "2\n");
}
TEST(InterpreterTest, UndefinedVar_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(printStmt(std::make_unique<VariableExpr>(
        Token{TokenType::IDENTIFIER, "notDef", std::monostate{}, 3})));
    Interpreter i;
    EXPECT_THROW(i.interpret(s), std::runtime_error);
}
TEST(InterpreterTest, BlockScope_Isolation) {
    Interpreter interp;
    std::vector<StmtPtr> s;
    std::vector<StmtPtr> inner;
    inner.push_back(varDecl("x", litStr("inner")));
    inner.push_back(printStmt(varRef("x")));
    s.push_back(blockStmt(std::move(inner)));
    s.push_back(printStmt(varRef("x")));
    EXPECT_THROW(interp.interpret(s), std::runtime_error);
}
TEST(InterpreterTest, IfTrue) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<IfStmt>(litBool(true), printStmt(litStr("yes")), nullptr));
    EXPECT_EQ(captureOutput([&]{ Interpreter().interpret(s); }), "yes\n");
}
TEST(InterpreterTest, IfFalse_GoesElse) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<IfStmt>(litBool(false), printStmt(litStr("no")), printStmt(litStr("yes"))));
    EXPECT_EQ(captureOutput([&]{ Interpreter().interpret(s); }), "yes\n");
}
TEST(InterpreterTest, ForLoop_0to2) {
    Interpreter interp;
    Token j  = makeIdent("j");
    Token lt = Token{TokenType::LESS,  "<", std::monostate{}, 1};
    Token pl = Token{TokenType::PLUS,  "+", std::monostate{}, 1};
    std::vector<StmtPtr> body;
    body.push_back(printStmt(std::make_unique<VariableExpr>(j)));
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ForStmt>(
        varDecl("j", litNum(0.0)),
        std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(j), lt, litNum(3.0)),
        std::make_unique<AssignExpr>(j,
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(j), pl, litNum(1.0))),
        blockStmt(std::move(body))));
    EXPECT_EQ(captureOutput([&]{ interp.interpret(s); }), "0\n1\n2\n");
}
