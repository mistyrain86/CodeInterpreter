#include <gtest/gtest.h>
#include "ConstantFolder.h"
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
    Interpreter m_interp;

    std::string run(StmtPtr stmt) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(std::move(stmt));
        return captureOutput([&]{ Interpreter().interpret(stmts); });
    }
    std::string runAll(std::vector<StmtPtr> stmts) {
        return captureOutput([&]{ m_interp.interpret(stmts); });
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

TEST_F(InterpreterFixture, Add)   { EXPECT_EQ(run(printStmt(bin(litNum(3),  TokenType::PLUS,  "+", litNum(4)))),  "7\n");  }
TEST_F(InterpreterFixture, Sub)   { EXPECT_EQ(run(printStmt(bin(litNum(10), TokenType::MINUS, "-", litNum(3)))),  "7\n");  }
TEST_F(InterpreterFixture, Mul)   { EXPECT_EQ(run(printStmt(bin(litNum(3),  TokenType::STAR,  "*", litNum(4)))),  "12\n"); }
TEST_F(InterpreterFixture, Div)   { EXPECT_EQ(run(printStmt(bin(litNum(8),  TokenType::SLASH, "/", litNum(2)))),  "4\n");  }
TEST_F(InterpreterFixture, StrConcat) {
    EXPECT_EQ(run(printStmt(bin(litStr("Hi"), TokenType::PLUS, "+", litStr("!")))), "Hi!\n");
}
TEST_F(InterpreterFixture, CmpLess_True) {
    EXPECT_EQ(run(printStmt(bin(litNum(1), TokenType::LESS,          "<",  litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, CmpLessEqual_True) {
    EXPECT_EQ(run(printStmt(bin(litNum(2), TokenType::LESS_EQUAL,    "<=", litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, CmpGreater_False) {
    EXPECT_EQ(run(printStmt(bin(litNum(3), TokenType::GREATER,       ">",  litNum(5)))), "false\n");
}
TEST_F(InterpreterFixture, CmpGreaterEqual_True) {
    EXPECT_EQ(run(printStmt(bin(litNum(5), TokenType::GREATER_EQUAL, ">=", litNum(5)))), "true\n");
}
TEST_F(InterpreterFixture, CmpEqualEqual_True) {
    EXPECT_EQ(run(printStmt(bin(litNum(3), TokenType::EQUAL_EQUAL,   "==", litNum(3)))), "true\n");
}
TEST_F(InterpreterFixture, CmpBangEqual_True) {
    EXPECT_EQ(run(printStmt(bin(litNum(1), TokenType::BANG_EQUAL,    "!=", litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, TypeMismatch_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ExprStmt>(bin(litNum(1), TokenType::PLUS, "+", litStr("HI"))));
    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}
TEST_F(InterpreterFixture, DivByZero_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ExprStmt>(bin(litNum(1), TokenType::SLASH, "/", litNum(0))));
    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}

TEST_F(InterpreterFixture, VarDeclAndUse) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("a", litNum(10.0)));
    s.push_back(printStmt(varRef("a")));
    EXPECT_EQ(runAll(std::move(s)), "10\n");
}
TEST_F(InterpreterFixture, Reassignment) {
    Token a = makeIdent("a");
    std::vector<StmtPtr> s;
    s.push_back(varDecl("a", litNum(1.0)));
    s.push_back(std::make_unique<ExprStmt>(
        std::make_unique<AssignExpr>(a, litNum(2.0))));
    s.push_back(printStmt(varRef("a")));
    EXPECT_EQ(runAll(std::move(s)), "2\n");
}
TEST_F(InterpreterFixture, UndefinedVar_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(printStmt(std::make_unique<VariableExpr>(
        Token{TokenType::IDENTIFIER, "notDef", std::monostate{}, 3})));
    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}
TEST_F(InterpreterFixture, BlockScope_Isolation) {
    std::vector<StmtPtr> s;
    std::vector<StmtPtr> inner;
    inner.push_back(varDecl("x", litStr("inner")));
    inner.push_back(printStmt(varRef("x")));
    s.push_back(blockStmt(std::move(inner)));
    s.push_back(printStmt(varRef("x")));
    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}
TEST_F(InterpreterFixture, IfTrue) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0, litBool(true), printStmt(litStr("yes")), nullptr)), "yes\n");
}
TEST_F(InterpreterFixture, IfFalse_GoesElse) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0, litBool(false), printStmt(litStr("no")), printStmt(litStr("yes")))), "yes\n");
}
TEST_F(InterpreterFixture, UnaryBang_OnNil) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::BANG, "!"),
            std::make_unique<LiteralExpr>(Value{std::monostate{}})))),
        "true\n");
}
TEST_F(InterpreterFixture, UnaryBang_OnZero) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::BANG, "!"), litNum(0.0)))),
        "true\n");
}
TEST_F(InterpreterFixture, UnaryBang_OnString) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(opTok(TokenType::BANG, "!"), litStr("hello")))),
        "false\n");
}
TEST_F(InterpreterFixture, GroupingExpr_Eval) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<GroupingExpr>(bin(litNum(3), TokenType::PLUS, "+", litNum(4))))),
        "7\n");
}
TEST_F(InterpreterFixture, VarDecl_NoInitializer) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<VarStmt>(makeIdent("x"), nullptr));
    s.push_back(printStmt(varRef("x")));
    EXPECT_EQ(runAll(std::move(s)), "null\n");
}
TEST_F(InterpreterFixture, EqualEqual_SameString) {
    EXPECT_EQ(run(printStmt(bin(litStr("a"), TokenType::EQUAL_EQUAL, "==", litStr("a")))), "true\n");
}
TEST_F(InterpreterFixture, BangEqual_DifferentTypes) {
    EXPECT_EQ(run(printStmt(bin(litNum(1), TokenType::BANG_EQUAL, "!=", litStr("1")))), "true\n");
}

TEST_F(InterpreterFixture, ForLoop_0to2) {
    Token j  = makeIdent("j");
    Token lt = Token{TokenType::LESS,  "<", std::monostate{}, 1};
    Token pl = Token{TokenType::PLUS,  "+", std::monostate{}, 1};
    std::vector<StmtPtr> body;
    body.push_back(printStmt(std::make_unique<VariableExpr>(j)));
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ForStmt>(0,
        varDecl("j", litNum(0.0)),
        std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(j), lt, litNum(3.0)),
        std::make_unique<AssignExpr>(j,
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(j), pl, litNum(1.0))),
        blockStmt(std::move(body))));
    EXPECT_EQ(runAll(std::move(s)), "0\n1\n2\n");
}

static Token retTok(int line = 1) {
    return Token{TokenType::KW_RETURN, "return", std::monostate{}, line};
}
static Token parenTok(int line = 1) {
    return Token{TokenType::RIGHT_PAREN, ")", std::monostate{}, line};
}

static std::unique_ptr<CallExpr> makeCall(
        const std::string& name,
        std::vector<ExprPtr> args) {
    return std::make_unique<CallExpr>(
        varRef(name), parenTok(), std::move(args));
}

TEST_F(InterpreterFixture, Function_NoParams_Call) {
    std::vector<StmtPtr> body;
    body.push_back(printStmt(litStr("hello func")));

    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("greet"), std::vector<Token>{}, std::move(body)));
    s.push_back(std::make_unique<ExprStmt>(makeCall("greet", {})));

    EXPECT_EQ(runAll(std::move(s)), "hello func\n");
}

TEST_F(InterpreterFixture, Function_Params_And_Return) {
    std::vector<Token> params = { makeIdent("a"), makeIdent("b") };
    std::vector<StmtPtr> body;
    body.push_back(std::make_unique<ReturnStmt>(
        retTok(), bin(varRef("a"), TokenType::PLUS, "+", varRef("b"))));

    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("add"), std::move(params), std::move(body)));

    std::vector<ExprPtr> args;
    args.push_back(litNum(3.0));
    args.push_back(litNum(7.0));
    s.push_back(printStmt(makeCall("add", std::move(args))));

    EXPECT_EQ(runAll(std::move(s)), "10\n");
}

TEST_F(InterpreterFixture, Function_NoReturn_ReturnsNil) {
    std::vector<StmtPtr> body;
    body.push_back(std::make_unique<ExprStmt>(litNum(42.0)));

    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("noop"), std::vector<Token>{}, std::move(body)));
    s.push_back(printStmt(makeCall("noop", {})));

    EXPECT_EQ(runAll(std::move(s)), "null\n");
}

TEST_F(InterpreterFixture, Function_Recursive_Factorial) {
    Token n    = makeIdent("n");
    Token le   = Token{TokenType::LESS_EQUAL, "<=", std::monostate{}, 1};
    Token star = Token{TokenType::STAR, "*", std::monostate{}, 1};
    Token minus= Token{TokenType::MINUS, "-", std::monostate{}, 1};

    std::vector<ExprPtr> recArgs;
    recArgs.push_back(std::make_unique<BinaryExpr>(
        std::make_unique<VariableExpr>(n), minus, litNum(1.0)));

    auto nTimesRec = std::make_unique<BinaryExpr>(
        std::make_unique<VariableExpr>(n), star,
        makeCall("fact", std::move(recArgs)));

    std::vector<StmtPtr> body;
    body.push_back(std::make_unique<IfStmt>(0,
        std::make_unique<BinaryExpr>(
            std::make_unique<VariableExpr>(n), le, litNum(1.0)),
        std::make_unique<ReturnStmt>(retTok(), litNum(1.0)),
        nullptr));
    body.push_back(std::make_unique<ReturnStmt>(retTok(), std::move(nTimesRec)));

    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("fact"), std::vector<Token>{n}, std::move(body)));

    std::vector<ExprPtr> args;
    args.push_back(litNum(5.0));
    s.push_back(printStmt(makeCall("fact", std::move(args))));

    EXPECT_EQ(runAll(std::move(s)), "120\n");
}

TEST_F(InterpreterFixture, Function_Closure_CapturesOuter) {
    std::vector<StmtPtr> body;
    body.push_back(std::make_unique<ReturnStmt>(retTok(), varRef("x")));

    std::vector<StmtPtr> s;
    s.push_back(varDecl("x", litNum(10.0)));
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("getX"), std::vector<Token>{}, std::move(body)));
    s.push_back(printStmt(makeCall("getX", {})));

    EXPECT_EQ(runAll(std::move(s)), "10\n");
}

TEST_F(InterpreterFixture, Function_CallNonCallable_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("x", litStr("hello")));
    s.push_back(std::make_unique<ExprStmt>(makeCall("x", std::vector<ExprPtr>{})));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, Function_ArityMismatch_Throws) {
    std::vector<Token> params = { makeIdent("a"), makeIdent("b"), makeIdent("c") };
    std::vector<StmtPtr> body;

    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("foo"), std::move(params), std::move(body)));

    std::vector<ExprPtr> args;
    args.push_back(litNum(1.0));
    args.push_back(litNum(2.0));
    s.push_back(std::make_unique<ExprStmt>(makeCall("foo", std::move(args))));

    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}

static Token bracketTok(int line = 1) {
    return Token{TokenType::LEFT_BRACKET, "[", std::monostate{}, line};
}

static ExprPtr arrayCreate(double size) {
    std::vector<ExprPtr> args;
    args.push_back(litNum(size));
    return makeCall("Array", std::move(args));
}

static ExprPtr indexGet(ExprPtr obj, ExprPtr idx) {
    return std::make_unique<IndexGetExpr>(
        std::move(obj), bracketTok(), std::move(idx));
}

static ExprPtr indexSet(ExprPtr obj, ExprPtr idx, ExprPtr val) {
    return std::make_unique<IndexSetExpr>(
        std::move(obj), bracketTok(), std::move(idx), std::move(val));
}

TEST_F(InterpreterFixture, Array_Create_And_Print) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(3.0)));
    s.push_back(printStmt(indexGet(varRef("arr"), litNum(0.0))));
    EXPECT_EQ(runAll(std::move(s)), "null\n");
}

TEST_F(InterpreterFixture, Array_Write_And_Read) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(3.0)));
    s.push_back(std::make_unique<ExprStmt>(
        indexSet(varRef("arr"), litNum(0.0), litNum(10.0))));
    s.push_back(std::make_unique<ExprStmt>(
        indexSet(varRef("arr"), litNum(1.0), litNum(20.0))));
    s.push_back(printStmt(indexGet(varRef("arr"), litNum(0.0))));
    s.push_back(printStmt(indexGet(varRef("arr"), litNum(1.0))));
    EXPECT_EQ(runAll(std::move(s)), "10\n20\n");
}

TEST_F(InterpreterFixture, Array_DynamicIndex) {
    Token iToken = makeIdent("i");
    Token minus  = Token{TokenType::MINUS, "-", std::monostate{}, 1};
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(3.0)));
    s.push_back(varDecl("i", litNum(2.0)));
    auto dynamicIdx = std::make_unique<BinaryExpr>(
        std::make_unique<VariableExpr>(iToken), minus, litNum(1.0));
    s.push_back(std::make_unique<ExprStmt>(
        indexSet(varRef("arr"), std::move(dynamicIdx), litNum(7.0))));
    s.push_back(printStmt(indexGet(varRef("arr"), litNum(1.0))));
    EXPECT_EQ(runAll(std::move(s)), "7\n");
}

TEST_F(InterpreterFixture, Array_OutOfBounds_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(2.0)));
    s.push_back(printStmt(indexGet(varRef("arr"), litNum(5.0))));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NegativeIndex_Throws) {
    Token minus = Token{TokenType::MINUS, "-", std::monostate{}, 1};
    auto negIdx = std::make_unique<UnaryExpr>(minus, litNum(1.0));
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(3.0)));
    s.push_back(printStmt(indexGet(varRef("arr"), std::move(negIdx))));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NonNumericIndex_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(3.0)));
    s.push_back(printStmt(indexGet(varRef("arr"), litStr("hello"))));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NonArrayTarget_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("x", litNum(10.0)));
    s.push_back(printStmt(indexGet(varRef("x"), litNum(0.0))));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NonNumericSize_Throws) {
    std::vector<StmtPtr> s;
    std::vector<ExprPtr> args;
    args.push_back(litStr("hi"));
    s.push_back(varDecl("arr", makeCall("Array", std::move(args))));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_TooLargeSize_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(1000001.0)));
    EXPECT_THROW(runAll(std::move(s)), RuntimeError);
}

TEST_F(InterpreterFixture, StaticBinding_Variable_SameResult) {
    auto varExpr = std::make_unique<VariableExpr>(makeIdent("x"));
    const VariableExpr* varPtr = varExpr.get();

    Interpreter::BindingMap bindings;
    bindings[varPtr] = 0;
    m_interp.setBindings(&bindings);

    std::vector<StmtPtr> s;
    s.push_back(varDecl("x", litNum(10.0)));
    s.push_back(printStmt(std::move(varExpr)));
    EXPECT_EQ(runAll(std::move(s)), "10\n");

    m_interp.setBindings(nullptr);
}

TEST_F(InterpreterFixture, StaticBinding_Assign_SameResult) {
    auto assignExpr = std::make_unique<AssignExpr>(makeIdent("x"), litNum(99.0));
    const AssignExpr* assignPtr = assignExpr.get();

    Interpreter::BindingMap bindings;
    bindings[assignPtr] = 0;
    m_interp.setBindings(&bindings);

    std::vector<StmtPtr> s;
    s.push_back(varDecl("x", litNum(1.0)));
    s.push_back(std::make_unique<ExprStmt>(std::move(assignExpr)));
    s.push_back(printStmt(varRef("x")));
    EXPECT_EQ(runAll(std::move(s)), "99\n");

    m_interp.setBindings(nullptr);
}

class ConstantFolderFixture : public ::testing::Test {
protected:
    ConstantFolder       m_folder;
    std::vector<StmtPtr> m_result;  // foldToDouble/wasFolded의 AST 수명 보장

    double foldToDouble(ExprPtr expr) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(printStmt(std::move(expr)));
        m_result = m_folder.optimize(std::move(stmts));
        auto* ps  = dynamic_cast<PrintStmt*>(m_result[0].get());
        auto* lit = dynamic_cast<LiteralExpr*>(ps->m_expression.get());
        EXPECT_NE(lit, nullptr) << "표현식이 LiteralExpr로 폴딩되지 않았습니다";
        if (!lit) return 0.0;
        return std::get<double>(lit->value);
    }

    bool wasFolded(ExprPtr expr) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(printStmt(std::move(expr)));
        m_result = m_folder.optimize(std::move(stmts));
        auto* ps = dynamic_cast<PrintStmt*>(m_result[0].get());
        return dynamic_cast<LiteralExpr*>(ps->m_expression.get()) != nullptr;
    }
};

TEST_F(ConstantFolderFixture, Fold_Plus) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(3),  TokenType::PLUS,  "+", litNum(4))),  7.0);
}
TEST_F(ConstantFolderFixture, Fold_Minus) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(10), TokenType::MINUS, "-", litNum(3))),  7.0);
}
TEST_F(ConstantFolderFixture, Fold_Star) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(3),  TokenType::STAR,  "*", litNum(4))), 12.0);
}
TEST_F(ConstantFolderFixture, Fold_Slash) {
    EXPECT_DOUBLE_EQ(foldToDouble(bin(litNum(8),  TokenType::SLASH, "/", litNum(2))),  4.0);
}
TEST_F(ConstantFolderFixture, NoFold_DivisionByZero) {
    EXPECT_FALSE(wasFolded(bin(litNum(1), TokenType::SLASH, "/", litNum(0))));
}
TEST_F(ConstantFolderFixture, NoFold_WithVariable) {
    EXPECT_FALSE(wasFolded(bin(varRef("x"), TokenType::PLUS, "+", litNum(1))));
}
TEST_F(ConstantFolderFixture, Fold_Nested) {
    auto inner = bin(litNum(1), TokenType::PLUS, "+", litNum(2));
    auto outer = bin(std::move(inner), TokenType::STAR, "*", litNum(3));
    EXPECT_DOUBLE_EQ(foldToDouble(std::move(outer)), 9.0);
}

TEST(ConstantFolderTest, Fold_VarStmt_Initializer) {
    ConstantFolder folder;
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", bin(litNum(2), TokenType::STAR, "*", litNum(3))));
    auto result = folder.optimize(std::move(stmts));
    auto* vs  = dynamic_cast<VarStmt*>(result[0].get());
    auto* lit = dynamic_cast<LiteralExpr*>(vs->m_initializer.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 6.0);
}

TEST_F(InterpreterFixture, ConstantFolder_RunResult) {
    ConstantFolder folder;
    auto expr = bin(
        bin(litNum(3), TokenType::PLUS, "+", litNum(4)),
        TokenType::STAR, "*", litNum(2));
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(std::move(expr)));
    auto folded = folder.optimize(std::move(stmts));
    EXPECT_EQ(runAll(std::move(folded)), "14\n");
}

TEST_F(InterpreterFixture, Truthy_Zero_IsFalse) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0,
        litNum(0.0), printStmt(litStr("yes")), nullptr)), "");
}

TEST_F(InterpreterFixture, Truthy_NonZero_IsTrue) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0,
        litNum(1.0), printStmt(litStr("yes")), nullptr)), "yes\n");
}

TEST_F(InterpreterFixture, Truthy_String_IsTrue) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0,
        litStr("hello"), printStmt(litStr("yes")), nullptr)), "yes\n");
}

TEST_F(InterpreterFixture, ForStmt_NullBody_Throws) {
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ForStmt>(0,
        nullptr, nullptr, nullptr, nullptr));
    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}
