#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "Checker.h"
#include "Interpreter.h"
#include "LangFactory.h"
#include "Lexer.h"
#include "Parser.h"
#include "Resolver.h"
#include "TestUtils.h"

using ::testing::_;

TEST(CheckerUnit, EmptyProgram_NoThrow) {
    EXPECT_NO_THROW(Checker().check({}));
}

TEST(CheckerUnit, GlobalVarDecl_NoThrow) {
    auto stmts = stmtList(varDecl("a", litNum(10.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, DuplicateLocalVar_Throws) {
    auto block = stmtList(
        varDecl("a", litStr("hi"), 1),
        varDecl("a", litNum(3.0), 2));
    auto stmts = stmtList(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

TEST(CheckerUnit, DuplicateLocal_ErrorContainsName) {
    auto block = stmtList(
        varDecl("myVar", litNum(1.0), 1),
        varDecl("myVar", litNum(2.0), 2));
    auto stmts = stmtList(blockStmt(std::move(block)));
    try { Checker().check(stmts); FAIL(); }
    catch (const CheckError& e) {
        EXPECT_NE(std::string(e.what()).find("myVar"), std::string::npos);
    }
}

TEST(CheckerUnit, SameName_DifferentScope_NoThrow) {
    auto inner = stmtList(varDecl("x", litNum(2.0)));
    auto stmts = stmtList(
        varDecl("x", litNum(1.0)),
        blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, NestedBlock_Shadowing_NoThrow) {
    auto inner = stmtList(varDecl("x", litNum(2.0)));
    auto outer = stmtList(
        varDecl("x", litNum(1.0)),
        blockStmt(std::move(inner)));
    auto stmts = stmtList(blockStmt(std::move(outer)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, DuplicateGlobal_Throws) {
    auto stmts = stmtList(
        varDecl("a", litNum(1.0)),
        varDecl("a", litNum(2.0)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

// 미선언 변수는 Checker가 전역 변수로 간주 — CheckError를 던지지 않음
// (런타임에서 RuntimeError로 처리)
TEST(CheckerUnit, UndeclaredVar_NoCheckError) {
    auto stmts = stmtList(
        varDecl("a", litNum(1.0)),
        printStmt(std::make_unique<VariableExpr>(makeIdent("x", 2))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, UndeclaredVar_NoCheckError_Single) {
    auto stmts = stmtList(
        printStmt(std::make_unique<VariableExpr>(makeIdent("missing", 1))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, SelfReferenceInInit_Throws) {
    Token a    = makeIdent("a", 1);
    auto block = stmtList(
        std::make_unique<VarStmt>(a, std::make_unique<VariableExpr>(a)));
    auto stmts = stmtList(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

TEST(CheckerUnit, ValidInit_NoThrow) {
    Token a    = makeIdent("a", 1);
    Token b    = makeIdent("b", 2);
    Token plus = makeToken(TokenType::PLUS, "+", 2);
    auto block = stmtList(
        varDecl("a", litNum(5.0), 1),
        std::make_unique<VarStmt>(b,
            std::make_unique<BinaryExpr>(
                std::make_unique<VariableExpr>(a), plus, litNum(1.0))));
    auto stmts = stmtList(blockStmt(std::move(block)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, NestedScope_OuterAccessible) {
    auto inner = stmtList(printStmt(std::make_unique<VariableExpr>(makeIdent("a"))));
    auto stmts = stmtList(
        varDecl("a", litNum(1.0)),
        blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ForLoopVar_InBody_NoThrow) {
    Token i  = makeIdent("i");
    Token lt = makeToken(TokenType::LESS, "<");
    Token pl = makeToken(TokenType::PLUS, "+");
    auto body  = stmtList(printStmt(std::make_unique<VariableExpr>(i)));
    auto stmts = stmtList(
        std::make_unique<ForStmt>(0,
            varDecl("i", litNum(0.0)),
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(i), lt, litNum(3.0)),
            std::make_unique<AssignExpr>(i,
                std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(i), pl, litNum(1.0))),
            blockStmt(std::move(body))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IfBranch_NoThrow) {
    auto thenB = stmtList(varDecl("x", litNum(1.0)));
    auto stmts = stmtList(
        std::make_unique<IfStmt>(0,
            litBool(true), blockStmt(std::move(thenB)), nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, VarNoInitializer_NoThrow) {
    auto stmts = stmtList(std::make_unique<VarStmt>(makeIdent("a", 1), nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IfElseBranch_NoThrow) {
    auto thenB = stmtList(varDecl("x", litNum(1.0)));
    auto elseB = stmtList(varDecl("y", litNum(2.0)));
    auto stmts = stmtList(
        std::make_unique<IfStmt>(0,
            litBool(true),
            blockStmt(std::move(thenB)),
            blockStmt(std::move(elseB))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ForAllNullFields_NoThrow) {
    auto stmts = stmtList(
        std::make_unique<ForStmt>(0, nullptr, nullptr, nullptr, nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ExprStmt_NoThrow) {
    Token plus = makeToken(TokenType::PLUS, "+");
    auto stmts = stmtList(std::make_unique<ExprStmt>(
        std::make_unique<BinaryExpr>(litNum(1.0), plus, litNum(2.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, FunctionNoParams_NoThrow) {
    auto stmts = stmtList(std::make_unique<FunctionStmt>(
        makeIdent("f", 1), std::vector<Token>{}, std::vector<StmtPtr>{}));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ReturnNoValue_InFunction_NoThrow) {
    auto body  = stmtList(std::make_unique<ReturnStmt>(makeIdent("return", 2), nullptr));
    auto stmts = stmtList(std::make_unique<FunctionStmt>(
        makeIdent("f", 1), std::vector<Token>{}, std::move(body)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, GroupingExpr_NoThrow) {
    auto stmts = stmtList(std::make_unique<ExprStmt>(
        std::make_unique<GroupingExpr>(litNum(1.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, UnaryExpr_NoThrow) {
    Token minus = makeToken(TokenType::MINUS, "-");
    auto stmts  = stmtList(std::make_unique<ExprStmt>(
        std::make_unique<UnaryExpr>(minus, litNum(1.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, CallExprNoArgs_NoThrow) {
    Token paren = makeToken(TokenType::RIGHT_PAREN, ")");
    auto stmts  = stmtList(
        varDecl("foo", litNum(1.0)),
        std::make_unique<ExprStmt>(
            std::make_unique<CallExpr>(
                std::make_unique<VariableExpr>(makeIdent("foo")),
                paren, std::vector<ExprPtr>{})));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, CallExprWithArgs_NoThrow) {
    Token paren = makeToken(TokenType::RIGHT_PAREN, ")");
    std::vector<ExprPtr> args;
    args.push_back(litNum(42.0));
    auto stmts = stmtList(
        varDecl("foo", litNum(1.0)),
        std::make_unique<ExprStmt>(
            std::make_unique<CallExpr>(
                std::make_unique<VariableExpr>(makeIdent("foo")),
                paren, std::move(args))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IndexGetExpr_NoThrow) {
    Token bracket = makeToken(TokenType::LEFT_BRACKET, "[");
    auto stmts    = stmtList(
        varDecl("arr", litNum(1.0)),
        std::make_unique<ExprStmt>(
            std::make_unique<IndexGetExpr>(
                std::make_unique<VariableExpr>(makeIdent("arr")),
                bracket, litNum(0.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IndexSetExpr_NoThrow) {
    Token bracket = makeToken(TokenType::LEFT_PAREN, "[");
    auto stmts    = stmtList(
        varDecl("arr", litNum(1.0)),
        std::make_unique<ExprStmt>(
            std::make_unique<IndexSetExpr>(
                std::make_unique<VariableExpr>(makeIdent("arr")),
                bracket, litNum(0.0), litNum(99.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

class CheckerMockFixture : public ::testing::Test {
protected:
    MockLexer*       m_mlRaw = nullptr;
    MockParser*      m_mpRaw = nullptr;
    MockInterpreter* m_miRaw = nullptr;
    std::unique_ptr<LangFactory> m_factory;

    void SetUp() override {
        auto ml  = std::make_unique<MockLexer>();
        m_mlRaw  = ml.get();
        EXPECT_CALL(*m_mlRaw, tokenize(_)).WillOnce(::testing::Return(std::vector<Token>{}));
        auto mp  = std::make_unique<MockParser>();
        m_mpRaw  = mp.get();
        auto mi  = std::make_unique<MockInterpreter>();
        m_miRaw  = mi.get();
        m_factory = std::make_unique<LangFactory>(
            std::move(ml), std::move(mp),
            std::make_unique<Checker>(), std::move(mi));
    }
};

TEST_F(CheckerMockFixture, DuplicateVar_MockParser_Throws) {
    EXPECT_CALL(*m_mpRaw, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> {
            auto block = stmtList(varDecl("a", litNum(1.0), 1), varDecl("a", litNum(2.0), 2));
            return stmtList(blockStmt(std::move(block)));
        }));
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(0);
    EXPECT_THROW(m_factory->run(""), CheckError);
}

TEST_F(CheckerMockFixture, ValidCode_MockParser_NoThrow) {
    EXPECT_CALL(*m_mpRaw, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> {
            return stmtList(varDecl("a", litNum(10.0)));
        }));
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(1);
    EXPECT_NO_THROW(m_factory->run(""));
}

class CheckerIntegrationFixture : public ::testing::Test {
protected:
    MockInterpreter* m_miRaw = nullptr;
    std::unique_ptr<LangFactory> m_factory;

    void SetUp() override {
        auto mi  = std::make_unique<MockInterpreter>();
        m_miRaw  = mi.get();
        m_factory = std::make_unique<LangFactory>(
            std::make_unique<Lexer>(), std::make_unique<Parser>(),
            std::make_unique<Checker>(), std::move(mi));
    }
};

TEST_F(CheckerIntegrationFixture, DuplicateVar_Throws) {
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(0);
    EXPECT_THROW(m_factory->run("{ var a = 1; var a = 2; }"), CheckError);
}

TEST_F(CheckerIntegrationFixture, ValidVarDecl_NoThrow) {
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(1);
    EXPECT_NO_THROW(m_factory->run("var a = 10;"));
}

TEST_F(CheckerIntegrationFixture, SelfRefInit_Throws) {
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(0);
    EXPECT_THROW(m_factory->run("{ var a = a; }"), CheckError);
}

TEST_F(CheckerIntegrationFixture, Shadowing_NoThrow) {
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(1);
    EXPECT_NO_THROW(m_factory->run("var x = 1; { var x = 2; }"));
}

class CheckerRealFixture : public ::testing::Test {
protected:
    Lexer    m_lexer;
    Parser   m_parser;
    Checker  m_checker;
    Resolver m_resolver;
};

TEST_F(CheckerRealFixture, DuplicateParam_Throws) {
    auto stmts = m_parser.parse(m_lexer.tokenize("func foo(a, a) { }"));
    EXPECT_THROW(m_checker.check(stmts), CheckError);
}

TEST_F(CheckerRealFixture, ReturnOutsideFunction_Throws) {
    auto stmts = m_parser.parse(m_lexer.tokenize("return 5;"));
    EXPECT_THROW(m_checker.check(stmts), CheckError);
}

TEST_F(CheckerRealFixture, ValidFunction_NoThrow) {
    auto stmts = m_parser.parse(m_lexer.tokenize("func add(a, b) { return a; }"));
    EXPECT_NO_THROW(m_checker.check(stmts));
}

TEST_F(CheckerRealFixture, GlobalVar_NotInBindings) {
    auto bindings = m_resolver.resolve(m_parser.parse(m_lexer.tokenize("var x = 1; print x;")));
    EXPECT_TRUE(bindings.empty());
}

TEST_F(CheckerRealFixture, LocalVar_InBindings) {
    auto bindings = m_resolver.resolve(m_parser.parse(m_lexer.tokenize("{ var x = 1; print x; }")));
    EXPECT_FALSE(bindings.empty());
}

TEST(CheckerUnit, ExprStmt_Variable_Checked) {
    Checker c;
    auto stmts = stmtList(
        varDecl("x", litNum(1.0)),
        std::make_unique<ExprStmt>(varRef("x")));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, UnaryExpr_Bang_Checked) {
    Checker c;
    auto stmts = stmtList(printStmt(
        std::make_unique<UnaryExpr>(makeToken(TokenType::BANG, "!"), litBool(true))));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, GroupingExpr_Checked) {
    Checker c;
    auto stmts = stmtList(printStmt(std::make_unique<GroupingExpr>(litNum(42.0))));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, LogicalExpr_And_NoThrow) {
    Checker c;
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        logicalExpr(litBool(true), TokenType::KW_AND, "and", litBool(false))));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, LogicalExpr_Or_NoThrow) {
    Checker c;
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        logicalExpr(litBool(false), TokenType::KW_OR, "or", litBool(true))));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, LogicalExpr_WithVars_NoThrow) {
    Checker c;
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", litBool(true)));
    stmts.push_back(varDecl("y", litBool(false)));
    stmts.push_back(printStmt(
        logicalExpr(varRef("x"), TokenType::KW_OR, "or", varRef("y"))));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, LogicalExpr_UndeclaredVar_NoThrow) {
    Checker c;
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        logicalExpr(litBool(true), TokenType::KW_AND, "and", varRef("undeclared"))));
    EXPECT_NO_THROW(c.check(stmts));
}
