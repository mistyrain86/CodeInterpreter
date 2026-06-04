#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "Checker.h"
#include "LangFactory.h"
#include "TestUtils.h"

using ::testing::_;

// 문장이 0개인 프로그램을 Checker에 넘겼을 때 예외가 발생하지 않아야 함
TEST(CheckerUnit, EmptyProgram_NoThrow) {
    EXPECT_NO_THROW(Checker().check({}));
}

//전역 스코프에서 변수 하나를 선언할 때 예외가 발생하지 않아야 함
TEST(CheckerUnit, GlobalVarDecl_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(10.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

//{
//    var a = "hi";   // line 1
//    var a = 3.0;    // line 2  ← 같은 블록 안에서 재선언
//}
//같은 블록(로컬 스코프) 안에서 동일한 이름으로 두 번 선언하면 CheckError가 throw

TEST(CheckerUnit, DuplicateLocalVar_Throws) {
    std::vector<StmtPtr> block;
    block.push_back(varDecl("a", litStr("hi"), 1));
    block.push_back(varDecl("a", litNum(3.0), 2));
    std::vector<StmtPtr> stmts;
    stmts.push_back(blockStmt(std::move(block)));
    EXPECT_THROW(Checker().check(stmts), CheckError);
}

//{
//    var myVar = 1.0;   // line 1
//    var myVar = 2.0;   // line 2  ← 중복
//}
//검증: TC 3과 동일한 시나리오지만, throw된 CheckError의 메시지에 변수명 "myVar"가 포함되어 있어야 함
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

//var x = 1.0;     // 외부(전역) 스코프
//{
//    var x = 2.0; // 내부(블록) 스코프 — 섀도잉
//}
//검증: 다른 스코프에서 같은 이름을 사용하는 것(섀도잉)은 에러가 아니어야 함

TEST(CheckerUnit, SameName_DifferentScope_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("x", litNum(1.0)));
    std::vector<StmtPtr> inner;
    inner.push_back(varDecl("x", litNum(2.0)));
    stmts.push_back(blockStmt(std::move(inner)));
    EXPECT_NO_THROW(Checker().check(stmts));
}

//var a = 1.0;   // 전역 스코프
//var a = 2.0;   // 전역 스코프에서 재선언
//
//검증: 전역 스코프에서 같은 이름이 두 번 선언되어도 예외가 발생하지 않아야 한다

TEST(CheckerUnit, DuplicateGlobal_NoThrow) {
    std::vector<StmtPtr> stmts;
    stmts.push_back(varDecl("a", litNum(1.0)));
    stmts.push_back(varDecl("a", litNum(2.0)));
    EXPECT_NO_THROW(Checker().check(stmts));
}