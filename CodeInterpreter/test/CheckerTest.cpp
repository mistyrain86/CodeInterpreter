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
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(10.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, DuplicateLocalVar_Throws) {
    std::vector<StmtPtr> block;
    block.push_back(varDecl("a", litStr("hi"), 1));
    block.push_back(varDecl("a", litNum(3.0), 2));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

TEST(CheckerUnit, DuplicateLocal_ErrorContainsName) {
    std::vector<StmtPtr> block;
    block.push_back(varDecl("myVar", litNum(1.0), 1));
    block.push_back(varDecl("myVar", litNum(2.0), 2));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    try { Checker().check(stmts); FAIL(); }
    catch (const CheckError& e) {
        EXPECT_NE(std::string(e.what()).find("myVar"), std::string::npos);
    }
}

TEST(CheckerUnit, SameName_DifferentScope_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", litNum(1.0)));
    std::vector<StmtPtr> inner;
    inner.push_back(varDecl("x", litNum(2.0)));
    stmts.push_back(blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, NestedBlock_Shadowing_NoThrow) {
    std::vector<StmtPtr> inner;
    inner.push_back(varDecl("x", litNum(2.0)));
    std::vector<StmtPtr> outer;
    outer.push_back(varDecl("x", litNum(1.0)));
    outer.push_back(blockStmt(std::move(inner)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(outer)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, DuplicateGlobal_Throws) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(1.0)));
    stmts.push_back(varDecl("a", litNum(2.0)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

TEST(CheckerUnit, UndeclaredVar_Throws) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(1.0)));
    stmts.push_back(printStmt(std::make_unique<VariableExpr>(makeIdent("x", 2))));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

TEST(CheckerUnit, UndeclaredVar_ErrorContainsName) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(std::make_unique<VariableExpr>(makeIdent("missing", 1))));
    try { Checker().check(stmts); FAIL(); }
    catch (const CheckError& e) {
        EXPECT_NE(std::string(e.what()).find("missing"), std::string::npos);
    }
}

TEST(CheckerUnit, SelfReferenceInInit_Throws) {
    Token a = makeIdent("a", 1);
    std::vector<StmtPtr> block;
    block.push_back(std::make_unique<VarStmt>(
        a, std::make_unique<VariableExpr>(a)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

TEST(CheckerUnit, ValidInit_NoThrow) {
    Token a    = makeIdent("a", 1);
    Token b    = makeIdent("b", 2);
    Token plus = Token{ TokenType::PLUS, "+", std::monostate{}, 2 };
    std::vector<StmtPtr> block;
    block.push_back(varDecl("a", litNum(5.0), 1));
    block.push_back(std::make_unique<VarStmt>(b,
        std::make_unique<BinaryExpr>(
            std::make_unique<VariableExpr>(a), plus, litNum(1.0))));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, NestedScope_OuterAccessible) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(1.0)));
    std::vector<StmtPtr> inner;
    inner.push_back(printStmt(std::make_unique<VariableExpr>(makeIdent("a"))));
    stmts.push_back(blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ForLoopVar_InBody_NoThrow) {
    Token i  = makeIdent("i");
    Token lt = Token{ TokenType::LESS, "<", std::monostate{}, 1 };
    Token pl = Token{ TokenType::PLUS, "+", std::monostate{}, 1 };
    std::vector<StmtPtr> body;
    body.push_back(printStmt(std::make_unique<VariableExpr>(i)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ForStmt>(0,
        varDecl("i", litNum(0.0)),
        std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(i), lt, litNum(3.0)),
        std::make_unique<AssignExpr>(i,
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(i), pl, litNum(1.0))),
        blockStmt(std::move(body))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IfBranch_NoThrow) {
    std::vector<StmtPtr> thenB;
    thenB.push_back(varDecl("x", litNum(1.0)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<IfStmt>(0,
        litBool(true), blockStmt(std::move(thenB)), nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, VarNoInitializer_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<VarStmt>(makeIdent("a", 1), nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IfElseBranch_NoThrow) {
    std::vector<StmtPtr> thenB, elseB;
    thenB.push_back(varDecl("x", litNum(1.0)));
    elseB.push_back(varDecl("y", litNum(2.0)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<IfStmt>(0,
        litBool(true), blockStmt(std::move(thenB)), blockStmt(std::move(elseB))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ForAllNullFields_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ForStmt>(0, nullptr, nullptr, nullptr, nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ExprStmt_NoThrow) {
    Token plus = Token{ TokenType::PLUS, "+", std::monostate{}, 1 };
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<BinaryExpr>(litNum(1.0), plus, litNum(2.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, FunctionNoParams_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<FunctionStmt>(
        makeIdent("f", 1), std::vector<Token>{}, std::vector<StmtPtr>{}));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, ReturnNoValue_InFunction_NoThrow) {
    std::vector<StmtPtr> body;
    body.push_back(std::make_unique<ReturnStmt>(makeIdent("return", 2), nullptr));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<FunctionStmt>(
        makeIdent("f", 1), std::vector<Token>{}, std::move(body)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, GroupingExpr_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<GroupingExpr>(litNum(1.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, UnaryExpr_NoThrow) {
    Token minus = Token{ TokenType::MINUS, "-", std::monostate{}, 1 };
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<UnaryExpr>(minus, litNum(1.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, CallExprNoArgs_NoThrow) {
    Token paren = Token{ TokenType::RIGHT_PAREN, ")", std::monostate{}, 1 };
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("foo", litNum(1.0)));
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<CallExpr>(
            std::make_unique<VariableExpr>(makeIdent("foo")),
            paren, std::vector<ExprPtr>{})));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, CallExprWithArgs_NoThrow) {
    Token paren = Token{ TokenType::RIGHT_PAREN, ")", std::monostate{}, 1 };
    std::vector<ExprPtr> args;
    args.push_back(litNum(42.0));
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("foo", litNum(1.0)));
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<CallExpr>(
            std::make_unique<VariableExpr>(makeIdent("foo")),
            paren, std::move(args))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IndexGetExpr_NoThrow) {
    Token bracket = Token{ TokenType::LEFT_BRACKET, "[", std::monostate{}, 1 };
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("arr", litNum(1.0)));
    stmts.push_back(std::make_unique<ExprStmt>(
        std::make_unique<IndexGetExpr>(
            std::make_unique<VariableExpr>(makeIdent("arr")),
            bracket, litNum(0.0))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

TEST(CheckerUnit, IndexSetExpr_NoThrow) {
    Token bracket = Token{ TokenType::LEFT_PAREN, "[", std::monostate{}, 1 };
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("arr", litNum(1.0)));
    stmts.push_back(std::make_unique<ExprStmt>(
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
            std::vector<StmtPtr> block;
            block.push_back(varDecl("a", litNum(1.0), 1));
            block.push_back(varDecl("a", litNum(2.0), 2));
            std::vector<StmtPtr> stmts;
            stmts.push_back(blockStmt(std::move(block)));
            return stmts;
        }));
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(0);
    EXPECT_THROW(m_factory->run(""), CheckError);
}

TEST_F(CheckerMockFixture, ValidCode_MockParser_NoThrow) {
    EXPECT_CALL(*m_mpRaw, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> {
            std::vector<StmtPtr> stmts;
            stmts.push_back(varDecl("a", litNum(10.0)));
            return stmts;
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
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", litNum(1.0)));
    stmts.push_back(std::make_unique<ExprStmt>(varRef("x")));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, UnaryExpr_Bang_Checked) {
    Checker c;
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        std::make_unique<UnaryExpr>(
            Token{TokenType::BANG, "!", std::monostate{}, 1},
            litBool(true))));
    EXPECT_NO_THROW(c.check(stmts));
}

TEST(CheckerUnit, GroupingExpr_Checked) {
    Checker c;
    std::vector<StmtPtr> stmts;
    stmts.push_back(printStmt(
        std::make_unique<GroupingExpr>(litNum(42.0))));
    EXPECT_NO_THROW(c.check(stmts));
}
