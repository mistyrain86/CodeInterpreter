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

// ── CheckerUnit: AST 직접 구성 단위 테스트 ───────────────────────

// (빈 프로그램) → 예외 없이 통과
TEST(CheckerUnit, EmptyProgram_NoThrow) {
    EXPECT_NO_THROW(Checker().check({}));
}

// var a = 10; → 전역 선언 정상 통과
TEST(CheckerUnit, GlobalVarDecl_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(10.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

// { var a = "hi"; var a = 3.0; } → 같은 블록 중복 선언 → CheckError
TEST(CheckerUnit, DuplicateLocalVar_Throws) {
    std::vector<StmtPtr> block;
    block.push_back(varDecl("a", litStr("hi"), 1));
    block.push_back(varDecl("a", litNum(3.0), 2));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

// { var myVar = 1.0; var myVar = 2.0; } → 에러 메시지에 변수명 포함
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

// var x = 1.0; { var x = 2.0; } → 전역-블록 섀도잉 허용
TEST(CheckerUnit, SameName_DifferentScope_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", litNum(1.0)));
    std::vector<StmtPtr> inner;
    inner.push_back(varDecl("x", litNum(2.0)));
    stmts.push_back(blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

// { var x = 1; { var x = 2; } } → 중첩 블록 섀도잉 허용
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

// var a = 1.0; var a = 2.0; → 전역 중복 선언 (현재 전역 스코프 미추적)
TEST(CheckerUnit, DuplicateGlobal_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(1.0)));
    stmts.push_back(varDecl("a", litNum(2.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

// { var a = a; } → 자기 참조 초기화 → CheckError
TEST(CheckerUnit, SelfReferenceInInit_Throws) {
    Token a = makeIdent("a", 1);
    std::vector<StmtPtr> block;
    block.push_back(std::make_unique<VarStmt>(
        a, std::make_unique<VariableExpr>(a)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

// { var a = 5.0; var b = a + 1.0; } → 정의된 변수 참조 정상 통과
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

// var a = 1.0; { print(a); } → 내부 블록에서 외부 변수 참조 허용
TEST(CheckerUnit, NestedScope_OuterAccessible) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(1.0)));
    std::vector<StmtPtr> inner;
    inner.push_back(printStmt(std::make_unique<VariableExpr>(makeIdent("a"))));
    stmts.push_back(blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

// for (var i = 0; i < 3; i = i + 1) { print(i); } → for 루프 변수 참조 정상
TEST(CheckerUnit, ForLoopVar_InBody_NoThrow) {
    Token i  = makeIdent("i");
    Token lt = Token{ TokenType::LESS, "<", std::monostate{}, 1 };
    Token pl = Token{ TokenType::PLUS, "+", std::monostate{}, 1 };
    std::vector<StmtPtr> body;
    body.push_back(printStmt(std::make_unique<VariableExpr>(i)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<ForStmt>(
        varDecl("i", litNum(0.0)),
        std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(i), lt, litNum(3.0)),
        std::make_unique<AssignExpr>(i,
            std::make_unique<BinaryExpr>(std::make_unique<VariableExpr>(i), pl, litNum(1.0))),
        blockStmt(std::move(body))));
    EXPECT_NO_THROW(Checker().check(stmts));
}

// if (true) { var x = 1.0; } → then 브랜치 내 변수 선언 정상
TEST(CheckerUnit, IfBranch_NoThrow) {
    std::vector<StmtPtr> thenB;
    thenB.push_back(varDecl("x", litNum(1.0)));
    std::vector<StmtPtr> stmts;
    stmts.push_back(std::make_unique<IfStmt>(
        litBool(true), blockStmt(std::move(thenB)), nullptr));
    EXPECT_NO_THROW(Checker().check(stmts));
}

// ── CheckerMock: Mock Lexer·Parser 통합 테스트 ───────────────────

// MockParser → { var a = 1; var a = 2; } → CheckError, Interpreter 미호출
TEST(CheckerMock, DuplicateVar_MockParser_Throws) {
    auto ml = std::make_unique<MockLexer>();
    EXPECT_CALL(*ml, tokenize(_)).WillOnce(::testing::Return(std::vector<Token>{}));
    auto mp = std::make_unique<MockParser>();
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> {
            std::vector<StmtPtr> block;
            block.push_back(varDecl("a", litNum(1.0), 1));
            block.push_back(varDecl("a", litNum(2.0), 2));
            std::vector<StmtPtr> stmts;
            stmts.push_back(blockStmt(std::move(block)));
            return stmts;
        }));
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    LangFactory factory(std::move(ml), std::move(mp),
        std::make_unique<Checker>(), std::move(mi));
    EXPECT_THROW(factory.run(""), CheckError);
}

// MockParser → var a = 10.0; → Checker 통과, Interpreter 1회 호출
TEST(CheckerMock, ValidCode_MockParser_NoThrow) {
    auto ml = std::make_unique<MockLexer>();
    EXPECT_CALL(*ml, tokenize(_)).WillOnce(::testing::Return(std::vector<Token>{}));
    auto mp = std::make_unique<MockParser>();
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> {
            std::vector<StmtPtr> stmts;
            stmts.push_back(varDecl("a", litNum(10.0)));
            return stmts;
        }));
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    LangFactory factory(std::move(ml), std::move(mp),
        std::make_unique<Checker>(), std::move(mi));
    EXPECT_NO_THROW(factory.run(""));
}

// ── CheckerRealParser: 실제 Lexer·Parser 통합 테스트 ─────────────

// { var a = 1; var a = 2; } → CheckError, Interpreter 미호출
TEST(CheckerRealParser, DuplicateVar_Throws) {
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
        std::make_unique<Checker>(), std::move(mi));
    EXPECT_THROW(factory.run("{ var a = 1; var a = 2; }"), CheckError);
}

// var a = 10; → Checker 통과, Interpreter 1회 호출
TEST(CheckerRealParser, ValidVarDecl_NoThrow) {
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
        std::make_unique<Checker>(), std::move(mi));
    EXPECT_NO_THROW(factory.run("var a = 10;"));
}

// { var a = a; } → 자기 참조 → CheckError, Interpreter 미호출
TEST(CheckerRealParser, SelfRefInit_Throws) {
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(0);
    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
        std::make_unique<Checker>(), std::move(mi));
    EXPECT_THROW(factory.run("{ var a = a; }"), CheckError);
}

// var x = 1; { var x = 2; } → 섀도잉 허용, Interpreter 1회 호출
TEST(CheckerRealParser, Shadowing_NoThrow) {
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);
    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
        std::make_unique<Checker>(), std::move(mi));
    EXPECT_NO_THROW(factory.run("var x = 1; { var x = 2; }"));
}

// ── CheckerTest: 함수·return 의미 검사 ───────────────────────────

// func foo(a, a) { } → 파라미터 중복 → CheckError
TEST(CheckerTest, DuplicateParam_Throws) {
    Lexer l; Parser p; Checker c;
    auto stmts = p.parse(l.tokenize("func foo(a, a) { }"));
    EXPECT_THROW(c.check(stmts), CheckError);
}

// return 5; → 함수 외부 return → CheckError
TEST(CheckerTest, ReturnOutsideFunction_Throws) {
    Lexer l; Parser p; Checker c;
    auto stmts = p.parse(l.tokenize("return 5;"));
    EXPECT_THROW(c.check(stmts), CheckError);
}

// func add(a, b) { return a; } → 정상 함수 선언 통과
TEST(CheckerTest, ValidFunction_NoThrow) {
    Lexer l; Parser p; Checker c;
    auto stmts = p.parse(l.tokenize("func add(a, b) { return a; }"));
    EXPECT_NO_THROW(c.check(stmts));
}

// ── ResolverTest: 변수 바인딩 거리 계산 ──────────────────────────

// var x = 1; print x; → 전역 변수는 BindingMap에 등록되지 않음
TEST(ResolverTest, GlobalVar_NotInBindings) {
    Lexer l; Parser p; Resolver r;
    auto bindings = r.resolve(p.parse(l.tokenize("var x = 1; print x;")));
    EXPECT_TRUE(bindings.empty());
}

// { var x = 1; print x; } → 로컬 변수는 BindingMap에 distance=0으로 등록
TEST(ResolverTest, LocalVar_InBindings) {
    Lexer l; Parser p; Resolver r;
    auto bindings = r.resolve(p.parse(l.tokenize("{ var x = 1; print x; }")));
    EXPECT_FALSE(bindings.empty());
}
