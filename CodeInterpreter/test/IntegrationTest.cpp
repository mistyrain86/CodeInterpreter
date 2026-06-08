#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include "LangFactory.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"

// 소스 코드를 실행하고 stdout 출력을 반환
static std::string exec(const std::string& source) {
    std::ostringstream oss;
    auto* old = std::cout.rdbuf(oss.rdbuf());
    LangFactory factory;
    factory.run(source);
    std::cout.rdbuf(old);
    return oss.str();
}

// 에러가 발생해야 하는 케이스 — 타입별 헬퍼
static void expectParseError(const std::string& source) {
    EXPECT_THROW(LangFactory().run(source), ParseError);
}
static void expectCheckError(const std::string& source) {
    EXPECT_THROW(LangFactory().run(source), CheckError);
}
static void expectRuntimeError(const std::string& source) {
    EXPECT_THROW(LangFactory().run(source), RuntimeError);
}

// ═══════════════════════════════════════════════════════════════════
// Ch.2 — Function 통합 테스트
// ═══════════════════════════════════════════════════════════════════

TEST(Ch2_Integration, BasicFunctionCallAndReturn) {
    // func add(a, b) { return a + b; }
    // var ret = add(3, 7); print ret;  → 10
    EXPECT_EQ(exec(
        "func add(a, b) { return a + b; }"
        "var ret = add(3, 7);"
        "print ret;"),
        "10\n");
}

TEST(Ch2_Integration, NoReturnReturnsNull) {
    // return 없는 함수 → null 출력
    EXPECT_EQ(exec(
        "func noop() { var x = 1; }"
        "print noop();"),
        "null\n");
}

TEST(Ch2_Integration, RecursiveFactorial) {
    // func fact(n) { if (n <= 1) return 1; return n * fact(n - 1); }
    // print fact(5);  → 120
    EXPECT_EQ(exec(
        "func fact(n) {"
        "  if (n <= 1) return 1;"
        "  return n * fact(n - 1);"
        "}"
        "print fact(5);"),
        "120\n");
}

TEST(Ch2_Integration, FunctionWithClosure) {
    // 클로저: 바깥 변수 캡처
    EXPECT_EQ(exec(
        "var x = 10;"
        "func getX() { return x; }"
        "print getX();"),
        "10\n");
}

TEST(Ch2_Integration, FunctionMultipleParams) {
    EXPECT_EQ(exec(
        "func mul(a, b, c) { return a * b * c; }"
        "print mul(2, 3, 4);"),
        "24\n");
}

// ── Ch.2 에러 케이스 ──────────────────────────────────────────────

TEST(Ch2_Integration, Error_ReturnOutsideFunction) {
    // return 5;  → 함수 외부 → CheckError
    expectCheckError("return 5;");
}

TEST(Ch2_Integration, Error_DuplicateParam) {
    // func foo(a, a) { }  → 파라미터 중복 → CheckError
    expectCheckError("func foo(a, a) { }");
}

TEST(Ch2_Integration, Error_CallNonCallable) {
    // var x = "hello"; x();  → RuntimeError
    expectRuntimeError("var x = \"hello\"; x();");
}

TEST(Ch2_Integration, Error_ArityMismatch) {
    // func foo(a, b, c) { }  foo(1, 2);  → 인자 불일치 → RuntimeError
    expectRuntimeError(
        "func foo(a, b, c) { }"
        "foo(1, 2);");
}

// ═══════════════════════════════════════════════════════════════════
// Ch.3 — 정적 배열 통합 테스트
// ═══════════════════════════════════════════════════════════════════

TEST(Ch3_Integration, ArrayCreateAndRead) {
    // var arr = Array(3); arr[0]=10; arr[1]=20; arr[2]=30; print arr[0]; → 10
    EXPECT_EQ(exec(
        "var arr = Array(3);"
        "arr[0] = 10; arr[1] = 20; arr[2] = 30;"
        "print arr[0];"),
        "10\n");
}

TEST(Ch3_Integration, ArrayPrintMultiple) {
    EXPECT_EQ(exec(
        "var arr = Array(3);"
        "arr[0] = 10; arr[1] = 20; arr[2] = 30;"
        "print arr[0]; print arr[1]; print arr[2];"),
        "10\n20\n30\n");
}

TEST(Ch3_Integration, ArrayDynamicIndex) {
    // var i = 2; arr[i-1] = 7;
    EXPECT_EQ(exec(
        "var arr = Array(3);"
        "var i = 2;"
        "arr[i - 1] = 7;"
        "print arr[1];"),
        "7\n");
}

TEST(Ch3_Integration, ArrayInitialValueIsNull) {
    // 생성 직후 원소는 null
    EXPECT_EQ(exec(
        "var arr = Array(3);"
        "print arr[0];"),
        "null\n");
}

// ── Ch.3 에러 케이스 ──────────────────────────────────────────────

TEST(Ch3_Integration, Error_OutOfBounds) {
    // print arr[5];  → 범위 초과 → RuntimeError
    expectRuntimeError(
        "var arr = Array(3);"
        "print arr[5];");
}

TEST(Ch3_Integration, Error_NonNumericIndex) {
    // arr["hello"]  → RuntimeError
    expectRuntimeError(
        "var arr = Array(3);"
        "print arr[\"hello\"];");
}

TEST(Ch3_Integration, Error_IndexOnNonArray) {
    // var x = 10; print x[0];  → RuntimeError
    expectRuntimeError("var x = 10; print x[0];");
}

TEST(Ch3_Integration, Error_NonNumericSize) {
    // var brr = Array("hi");  → RuntimeError
    expectRuntimeError("var brr = Array(\"hi\");");
}

TEST(Ch3_Integration, Error_NegativeSize) {
    // var arr = Array(-1);  → RuntimeError
    expectRuntimeError("var arr = Array(-1);");
}

// ═══════════════════════════════════════════════════════════════════
// Ch.4 — 최적화 통합 테스트
// ═══════════════════════════════════════════════════════════════════

TEST(Ch4_Integration, ConstantFolding_ArithResult) {
    // 상수 폴딩: 1 + 2 * 3 은 실행 전 7로 교체되어야 함
    EXPECT_EQ(exec("print 1 + 2 * 3;"), "7\n");
}

TEST(Ch4_Integration, ConstantFolding_NestedExpression) {
    // 1 - (2*3) + 7 + 8 + 9 = 1 - 6 + 24 = 19
    EXPECT_EQ(exec("print 1 - 2 * 3 + 7 + 8 + 9;"), "19\n");
}

TEST(Ch4_Integration, StaticBinding_LocalVar) {
    // 블록 내 지역 변수 — Resolver가 distance 계산
    EXPECT_EQ(exec(
        "{"
        "  var a = 10;"
        "  print a;"
        "}"),
        "10\n");
}

TEST(Ch4_Integration, StaticBinding_NestedScope) {
    // 중첩 스코프 변수 참조
    EXPECT_EQ(exec(
        "var outer = 1;"
        "{"
        "  var inner = 2;"
        "  print outer + inner;"
        "}"),
        "3\n");
}

TEST(Ch4_Integration, StaticBinding_DeepNested) {
    // 깊은 중첩 스코프에서 외부 변수 참조
    EXPECT_EQ(exec(
        "var a = 0;"
        "{"
        "  {"
        "    {"
        "      a = a + 1;"
        "    }"
        "  }"
        "}"
        "print a;"),
        "1\n");
}

// ═══════════════════════════════════════════════════════════════════
// 복합 통합 테스트 (Ch.2 + Ch.3 + Ch.4 함께)
// ═══════════════════════════════════════════════════════════════════

TEST(Integration_Combined, FunctionWithArray) {
    // 함수와 배열을 함께 사용
    EXPECT_EQ(exec(
        "func sumArray(arr, n) {"
        "  var total = 0;"
        "  for (var i = 0; i < n; i = i + 1) {"
        "    total = total + arr[i];"
        "  }"
        "  return total;"
        "}"
        "var arr = Array(3);"
        "arr[0] = 10; arr[1] = 20; arr[2] = 30;"
        "print sumArray(arr, 3);"),
        "60\n");
}

TEST(Integration_Combined, RecursiveWithConstantFolding) {
    // 재귀 함수 + 상수 폴딩
    EXPECT_EQ(exec(
        "func add(a, b) { return a + b; }"
        "print add(3 + 4, 2 * 5);"),  // 3+4=7, 2*5=10 폴딩 후 add(7,10)=17
        "17\n");
}

TEST(Integration_Combined, ArrayInLoop) {
    // 배열 + for 루프
    EXPECT_EQ(exec(
        "var arr = Array(3);"
        "for (var i = 0; i < 3; i = i + 1) {"
        "  arr[i] = i * 2;"
        "}"
        "print arr[0]; print arr[1]; print arr[2];"),
        "0\n2\n4\n");
}
