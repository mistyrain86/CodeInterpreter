#include <gtest/gtest.h>
#include "Interpreter.h"
#include "TestUtils.h"

static Token opTok(TokenType t, std::string lex, int line = 1) {
    return Token{t, std::move(lex), std::monostate{}, line};
}

class InterpreterFixture : public ::testing::Test {
protected:
    Interpreter m_interp;

    std::string run(StmtPtr stmt) {
        std::vector<StmtPtr> stmts;
        stmts.push_back(std::move(stmt));
        return captureOutput([&]{ m_interp.interpret(stmts); });
    }
    std::string runAll(std::vector<StmtPtr> stmts) {
        return captureOutput([&]{ m_interp.interpret(stmts); });
    }
};

TEST_F(InterpreterFixture, PrintInteger)   { EXPECT_EQ(run(printStmt(litNum(5.0))),       "5\n");     }
TEST_F(InterpreterFixture, PrintFloat)     { EXPECT_EQ(run(printStmt(litNum(3.14))),      "3.14\n");  }
TEST_F(InterpreterFixture, PrintString)    { EXPECT_EQ(run(printStmt(litStr("hello"))),   "hello\n"); }
TEST_F(InterpreterFixture, PrintBoolTrue)  { EXPECT_EQ(run(printStmt(litBool(true))),     "true\n");  }
TEST_F(InterpreterFixture, PrintBoolFalse) { EXPECT_EQ(run(printStmt(litBool(false))),    "false\n"); }

TEST_F(InterpreterFixture, UnaryMinus) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(makeToken(TokenType::MINUS, "-"), litNum(3.0)))),
        "-3\n");
}
TEST_F(InterpreterFixture, UnaryBang_True) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(makeToken(TokenType::BANG, "!"), litBool(true)))),
        "false\n");
}
TEST_F(InterpreterFixture, UnaryMinus_OnString_Throws) {
    Interpreter interp;
    auto stmts = stmtList(std::make_unique<ExprStmt>(
        std::make_unique<UnaryExpr>(makeToken(TokenType::MINUS, "-"), litStr("oops"))));
    EXPECT_THROW(interp.interpret(stmts), RuntimeError);
}

TEST_F(InterpreterFixture, Add)   { EXPECT_EQ(run(printStmt(binaryExpr(litNum(3),  TokenType::PLUS,  "+", litNum(4)))),  "7\n");  }
TEST_F(InterpreterFixture, Sub)   { EXPECT_EQ(run(printStmt(binaryExpr(litNum(10), TokenType::MINUS, "-", litNum(3)))),  "7\n");  }
TEST_F(InterpreterFixture, Mul)   { EXPECT_EQ(run(printStmt(binaryExpr(litNum(3),  TokenType::STAR,  "*", litNum(4)))),  "12\n"); }
TEST_F(InterpreterFixture, Div)   { EXPECT_EQ(run(printStmt(binaryExpr(litNum(8),  TokenType::SLASH, "/", litNum(2)))),  "4\n");  }
TEST_F(InterpreterFixture, StrConcat) {
    EXPECT_EQ(run(printStmt(binaryExpr(litStr("Hi"), TokenType::PLUS, "+", litStr("!")))), "Hi!\n");
}
TEST_F(InterpreterFixture, CmpLess_True) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(1), TokenType::LESS,          "<",  litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, CmpLessEqual_True) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(2), TokenType::LESS_EQUAL,    "<=", litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, CmpGreater_False) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(3), TokenType::GREATER,       ">",  litNum(5)))), "false\n");
}
TEST_F(InterpreterFixture, CmpGreaterEqual_True) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(5), TokenType::GREATER_EQUAL, ">=", litNum(5)))), "true\n");
}
TEST_F(InterpreterFixture, CmpEqualEqual_True) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(3), TokenType::EQUAL_EQUAL,   "==", litNum(3)))), "true\n");
}
TEST_F(InterpreterFixture, CmpBangEqual_True) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(1), TokenType::BANG_EQUAL,    "!=", litNum(2)))), "true\n");
}
TEST_F(InterpreterFixture, TypeMismatch_Throws) {
    auto stmts = stmtList(
        std::make_unique<ExprStmt>(binaryExpr(litNum(1), TokenType::PLUS, "+", litStr("HI"))));
    EXPECT_THROW(m_interp.interpret(stmts), RuntimeError);
}
TEST_F(InterpreterFixture, DivByZero_Throws) {
    auto stmts = stmtList(
        std::make_unique<ExprStmt>(binaryExpr(litNum(1), TokenType::SLASH, "/", litNum(0))));
    EXPECT_THROW(m_interp.interpret(stmts), RuntimeError);
}

TEST_F(InterpreterFixture, VarDeclAndUse) {
    auto stmts = stmtList(
        varDecl("a", litNum(10.0)),
        printStmt(varRef("a")));
    EXPECT_EQ(runAll(std::move(stmts)), "10\n");
}
TEST_F(InterpreterFixture, Reassignment) {
    Token a    = makeIdent("a");
    auto stmts = stmtList(
        varDecl("a", litNum(1.0)),
        std::make_unique<ExprStmt>(std::make_unique<AssignExpr>(a, litNum(2.0))),
        printStmt(varRef("a")));
    EXPECT_EQ(runAll(std::move(stmts)), "2\n");
}
TEST_F(InterpreterFixture, UndefinedVar_Throws) {
    auto stmts = stmtList(
        printStmt(std::make_unique<VariableExpr>(makeIdent("notDef", 3))));
    EXPECT_THROW(m_interp.interpret(stmts), RuntimeError);
}
TEST_F(InterpreterFixture, BlockScope_Isolation) {
    auto inner = stmtList(
        varDecl("x", litStr("inner")),
        printStmt(varRef("x")));
    auto stmts = stmtList(
        blockStmt(std::move(inner)),
        printStmt(varRef("x")));
    captureOutput([&]{ EXPECT_THROW(m_interp.interpret(stmts), RuntimeError); });
}
TEST_F(InterpreterFixture, IfTrue) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0, litBool(true), printStmt(litStr("yes")), nullptr)), "yes\n");
}
TEST_F(InterpreterFixture, IfFalse_GoesElse) {
    EXPECT_EQ(run(std::make_unique<IfStmt>(0, litBool(false), printStmt(litStr("no")), printStmt(litStr("yes")))), "yes\n");
}
TEST_F(InterpreterFixture, UnaryBang_OnNil) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(makeToken(TokenType::BANG, "!"),
            std::make_unique<LiteralExpr>(Value{std::monostate{}})))),
        "true\n");
}
TEST_F(InterpreterFixture, UnaryBang_OnZero) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(makeToken(TokenType::BANG, "!"), litNum(0.0)))),
        "true\n");
}
TEST_F(InterpreterFixture, UnaryBang_OnString) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<UnaryExpr>(makeToken(TokenType::BANG, "!"), litStr("hello")))),
        "false\n");
}
TEST_F(InterpreterFixture, GroupingExpr_Eval) {
    EXPECT_EQ(run(printStmt(
        std::make_unique<GroupingExpr>(binaryExpr(litNum(3), TokenType::PLUS, "+", litNum(4))))),
        "7\n");
}
TEST_F(InterpreterFixture, VarDecl_NoInitializer) {
    auto stmts = stmtList(
        std::make_unique<VarStmt>(makeIdent("x"), nullptr),
        printStmt(varRef("x")));
    EXPECT_EQ(runAll(std::move(stmts)), "null\n");
}
TEST_F(InterpreterFixture, EqualEqual_SameString) {
    EXPECT_EQ(run(printStmt(binaryExpr(litStr("a"), TokenType::EQUAL_EQUAL, "==", litStr("a")))), "true\n");
}
TEST_F(InterpreterFixture, BangEqual_DifferentTypes) {
    EXPECT_EQ(run(printStmt(binaryExpr(litNum(1), TokenType::BANG_EQUAL, "!=", litStr("1")))), "true\n");
}

TEST_F(InterpreterFixture, ForLoop_0to2) {
    Token j  = makeIdent("j");
    Token lt = makeToken(TokenType::LESS, "<");
    Token pl = makeToken(TokenType::PLUS, "+");
    auto body  = stmtList(printStmt(std::make_unique<VariableExpr>(j)));
    auto stmts = stmtList(
        std::make_unique<ForStmt>(0,
            varDecl("j", litNum(0.0)),
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(j), lt, litNum(3.0)),
            std::make_unique<AssignExpr>(j,
                std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(j), pl, litNum(1.0))),
            blockStmt(std::move(body))));
    EXPECT_EQ(runAll(std::move(stmts)), "0\n1\n2\n");
}

static Token retTok(int line = 1) {
    return makeToken(TokenType::KW_RETURN, "return", line);
}
static Token parenTok(int line = 1) {
    return makeToken(TokenType::RIGHT_PAREN, ")", line);
}

static std::unique_ptr<CallExpr> makeCall(
        const std::string& name,
        std::vector<ExprPtr> args) {
    return std::make_unique<CallExpr>(
        varRef(name), parenTok(), std::move(args));
}

TEST_F(InterpreterFixture, Function_NoParams_Call) {
    auto body  = stmtList(printStmt(litStr("hello func")));
    auto stmts = stmtList(
        std::make_unique<FunctionStmt>(makeIdent("greet"), std::vector<Token>{}, std::move(body)),
        std::make_unique<ExprStmt>(makeCall("greet", {})));
    EXPECT_EQ(runAll(std::move(stmts)), "hello func\n");
}

TEST_F(InterpreterFixture, Function_Params_And_Return) {
    std::vector<Token> params = { makeIdent("a"), makeIdent("b") };
    std::vector<ExprPtr> args;
    args.push_back(litNum(3.0));
    args.push_back(litNum(7.0));
    auto body  = stmtList(std::make_unique<ReturnStmt>(
        retTok(), binaryExpr(varRef("a"), TokenType::PLUS, "+", varRef("b"))));
    auto stmts = stmtList(
        std::make_unique<FunctionStmt>(makeIdent("add"), std::move(params), std::move(body)),
        printStmt(makeCall("add", std::move(args))));
    EXPECT_EQ(runAll(std::move(stmts)), "10\n");
}

TEST_F(InterpreterFixture, Function_NoReturn_ReturnsNil) {
    auto body  = stmtList(std::make_unique<ExprStmt>(litNum(42.0)));
    auto stmts = stmtList(
        std::make_unique<FunctionStmt>(makeIdent("noop"), std::vector<Token>{}, std::move(body)),
        printStmt(makeCall("noop", {})));
    EXPECT_EQ(runAll(std::move(stmts)), "null\n");
}

TEST_F(InterpreterFixture, Function_Recursive_Factorial) {
    Token n     = makeIdent("n");
    Token le    = makeToken(TokenType::LESS_EQUAL, "<=");
    Token star  = makeToken(TokenType::STAR, "*");
    Token minus = makeToken(TokenType::MINUS, "-");

    std::vector<ExprPtr> recArgs;
    recArgs.push_back(std::make_unique<BinaryExpr>(
        std::make_unique<VariableExpr>(n), minus, litNum(1.0)));

    auto nTimesRec = std::make_unique<BinaryExpr>(
        std::make_unique<VariableExpr>(n), star,
        makeCall("fact", std::move(recArgs)));

    auto body  = stmtList(
        std::make_unique<IfStmt>(0,
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(n), le, litNum(1.0)),
            std::make_unique<ReturnStmt>(retTok(), litNum(1.0)),
            nullptr),
        std::make_unique<ReturnStmt>(retTok(), std::move(nTimesRec)));


    std::vector<ExprPtr> args;
    args.push_back(litNum(5.0));
    auto stmts = stmtList(
        std::make_unique<FunctionStmt>(makeIdent("fact"), std::vector<Token>{n}, std::move(body)),
        printStmt(makeCall("fact", std::move(args))));
    EXPECT_EQ(runAll(std::move(stmts)), "120\n");
}

TEST_F(InterpreterFixture, Function_Closure_CapturesOuter) {
    auto body  = stmtList(std::make_unique<ReturnStmt>(retTok(), varRef("x")));
    auto stmts = stmtList(
        varDecl("x", litNum(10.0)),
        std::make_unique<FunctionStmt>(makeIdent("getX"), std::vector<Token>{}, std::move(body)),
        printStmt(makeCall("getX", {})));
    EXPECT_EQ(runAll(std::move(stmts)), "10\n");
}


TEST_F(InterpreterFixture, Function_CallNonCallable_Throws) {
    auto stmts = stmtList(
        varDecl("x", litStr("hello")),
        std::make_unique<ExprStmt>(makeCall("x", std::vector<ExprPtr>{})));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, Function_ArityMismatch_Throws) {
    std::vector<Token> params = { makeIdent("a"), makeIdent("b"), makeIdent("c") };
    std::vector<ExprPtr> args;
    args.push_back(litNum(1.0));
    args.push_back(litNum(2.0));
    auto stmts = stmtList(
        std::make_unique<FunctionStmt>(
            makeIdent("foo"), std::move(params), std::vector<StmtPtr>{}),
        std::make_unique<ExprStmt>(makeCall("foo", std::move(args))));
    EXPECT_THROW(m_interp.interpret(stmts), RuntimeError);
}

static Token bracketTok(int line = 1) {
    return makeToken(TokenType::LEFT_BRACKET, "[", line);
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
    auto stmts = stmtList(
        varDecl("arr", arrayCreate(3.0)),
        printStmt(indexGet(varRef("arr"), litNum(0.0))));
    EXPECT_EQ(runAll(std::move(stmts)), "null\n");
}

TEST_F(InterpreterFixture, Array_Write_And_Read) {
    auto stmts = stmtList(
        varDecl("arr", arrayCreate(3.0)),
        std::make_unique<ExprStmt>(indexSet(varRef("arr"), litNum(0.0), litNum(10.0))),
        std::make_unique<ExprStmt>(indexSet(varRef("arr"), litNum(1.0), litNum(20.0))),
        printStmt(indexGet(varRef("arr"), litNum(0.0))),
        printStmt(indexGet(varRef("arr"), litNum(1.0))));
    EXPECT_EQ(runAll(std::move(stmts)), "10\n20\n");
}

TEST_F(InterpreterFixture, Array_DynamicIndex) {
    Token iToken = makeIdent("i");
    Token minus  = makeToken(TokenType::MINUS, "-");
    auto dynamicIdx = std::make_unique<BinaryExpr>(
        std::make_unique<VariableExpr>(iToken), minus, litNum(1.0));
    auto stmts = stmtList(
        varDecl("arr", arrayCreate(3.0)),
        varDecl("i", litNum(2.0)),
        std::make_unique<ExprStmt>(
            indexSet(varRef("arr"), std::move(dynamicIdx), litNum(7.0))),
        printStmt(indexGet(varRef("arr"), litNum(1.0))));
    EXPECT_EQ(runAll(std::move(stmts)), "7\n");
}

TEST_F(InterpreterFixture, Array_OutOfBounds_Throws) {
    auto stmts = stmtList(
        varDecl("arr", arrayCreate(2.0)),
        printStmt(indexGet(varRef("arr"), litNum(5.0))));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NegativeIndex_Throws) {
    Token minus = makeToken(TokenType::MINUS, "-");
    auto negIdx = std::make_unique<UnaryExpr>(minus, litNum(1.0));
    auto stmts  = stmtList(
        varDecl("arr", arrayCreate(3.0)),
        printStmt(indexGet(varRef("arr"), std::move(negIdx))));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NonNumericIndex_Throws) {
    auto stmts = stmtList(
        varDecl("arr", arrayCreate(3.0)),
        printStmt(indexGet(varRef("arr"), litStr("hello"))));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NonArrayTarget_Throws) {
    auto stmts = stmtList(
        varDecl("x", litNum(10.0)),
        printStmt(indexGet(varRef("x"), litNum(0.0))));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_NonNumericSize_Throws) {
    std::vector<ExprPtr> args;
    args.push_back(litStr("hi"));
    auto stmts = stmtList(
        varDecl("arr", makeCall("Array", std::move(args))));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, Array_TooLargeSize_Throws) {
    auto stmts = stmtList(varDecl("arr", arrayCreate(1000001.0)));
    EXPECT_THROW(runAll(std::move(stmts)), RuntimeError);
}

TEST_F(InterpreterFixture, StaticBinding_Variable_SameResult) {
    auto varExpr = std::make_unique<VariableExpr>(makeIdent("x"));
    const VariableExpr* varPtr = varExpr.get();

    Interpreter::BindingMap bindings;
    bindings[varPtr] = 0;
    m_interp.setBindings(&bindings);

    auto stmts = stmtList(
        varDecl("x", litNum(10.0)),
        printStmt(std::move(varExpr)));
    EXPECT_EQ(runAll(std::move(stmts)), "10\n");

    m_interp.setBindings(nullptr);
}

TEST_F(InterpreterFixture, StaticBinding_Assign_SameResult) {
    auto assignExpr = std::make_unique<AssignExpr>(makeIdent("x"), litNum(99.0));
    const AssignExpr* assignPtr = assignExpr.get();

    Interpreter::BindingMap bindings;
    bindings[assignPtr] = 0;
    m_interp.setBindings(&bindings);

    auto stmts = stmtList(
        varDecl("x", litNum(1.0)),
        std::make_unique<ExprStmt>(std::move(assignExpr)),
        printStmt(varRef("x")));
    EXPECT_EQ(runAll(std::move(stmts)), "99\n");

    m_interp.setBindings(nullptr);
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
    auto stmts = stmtList(
        std::make_unique<ForStmt>(0, nullptr, nullptr, nullptr, nullptr));
    EXPECT_THROW(m_interp.interpret(stmts), RuntimeError);
}

TEST_F(InterpreterFixture, Percent_Modulo) {
    Token pct = Token{TokenType::PERCENT, "%", std::monostate{}, 1};
    EXPECT_EQ(run(printStmt(
        std::make_unique<BinaryExpr>(litNum(10.0), pct, litNum(3.0)))),
        "1\n");
}

TEST_F(InterpreterFixture, Percent_ModuloByZero_Throws) {
    Token pct = Token{TokenType::PERCENT, "%", std::monostate{}, 1};
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<ExprStmt>(
        std::make_unique<BinaryExpr>(litNum(10.0), pct, litNum(0.0))));
    EXPECT_THROW(m_interp.interpret(s), RuntimeError);
}

TEST_F(InterpreterFixture, Stringify_Array_Print) {
    std::vector<StmtPtr> s;
    s.push_back(varDecl("arr", arrayCreate(3.0)));
    s.push_back(printStmt(varRef("arr")));
    EXPECT_EQ(runAll(std::move(s)), "[null, null, null]\n");
}

TEST_F(InterpreterFixture, Stringify_Function_PrintsFnName) {
    std::vector<StmtPtr> body;
    std::vector<StmtPtr> s;
    s.push_back(std::make_unique<FunctionStmt>(
        makeIdent("greet"), std::vector<Token>{}, std::move(body)));
    s.push_back(printStmt(varRef("greet")));
    EXPECT_EQ(runAll(std::move(s)), "<fn greet>\n");
}

TEST_F(InterpreterFixture, And_BothTruthy_ReturnsTrue) {
    EXPECT_EQ(run(printStmt(
        logicalExpr(litNum(1.0), TokenType::KW_AND, "and", litNum(2.0)))),
        "true\n");
}
TEST_F(InterpreterFixture, And_LeftFalsy_ReturnsFalse) {
    EXPECT_EQ(run(printStmt(
        logicalExpr(litBool(false), TokenType::KW_AND, "and", litNum(42.0)))),
        "false\n");
}
TEST_F(InterpreterFixture, And_RightFalsy_ReturnsFalse) {
    EXPECT_EQ(run(printStmt(
        logicalExpr(litNum(1.0), TokenType::KW_AND, "and", litBool(false)))),
        "false\n");
}
TEST_F(InterpreterFixture, And_ShortCircuit_SkipsRight) {
    std::vector<StmtPtr> s;
    s.push_back(printStmt(
        logicalExpr(litBool(false), TokenType::KW_AND, "and", varRef("undeclared"))));
    EXPECT_EQ(runAll(std::move(s)), "false\n");
}
TEST_F(InterpreterFixture, Or_LeftTruthy_ReturnsTrue) {
    EXPECT_EQ(run(printStmt(
        logicalExpr(litNum(1.0), TokenType::KW_OR, "or", litNum(2.0)))),
        "true\n");
}
TEST_F(InterpreterFixture, Or_LeftFalsy_RightTruthy_ReturnsTrue) {
    EXPECT_EQ(run(printStmt(
        logicalExpr(litBool(false), TokenType::KW_OR, "or", litNum(42.0)))),
        "true\n");
}
TEST_F(InterpreterFixture, Or_BothFalsy_ReturnsFalse) {
    EXPECT_EQ(run(printStmt(
        logicalExpr(litBool(false), TokenType::KW_OR, "or", litBool(false)))),
        "false\n");
}
TEST_F(InterpreterFixture, Or_ShortCircuit_SkipsRight) {
    std::vector<StmtPtr> s;
    s.push_back(printStmt(
        logicalExpr(litNum(1.0), TokenType::KW_OR, "or", varRef("undeclared"))));
    EXPECT_EQ(runAll(std::move(s)), "true\n");
}
TEST_F(InterpreterFixture, And_Chained_AllTruthy) {
    auto lhs = logicalExpr(litBool(true), TokenType::KW_AND, "and", litBool(true));
    EXPECT_EQ(run(printStmt(
        logicalExpr(std::move(lhs), TokenType::KW_AND, "and", litBool(true)))),
        "true\n");
}
TEST_F(InterpreterFixture, Or_Chained_FirstTruthy) {
    auto lhs = logicalExpr(litBool(false), TokenType::KW_OR, "or", litBool(false));
    EXPECT_EQ(run(printStmt(
        logicalExpr(std::move(lhs), TokenType::KW_OR, "or", litBool(true)))),
        "true\n");
}
