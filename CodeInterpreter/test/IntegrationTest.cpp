#include <gtest/gtest.h>
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#include "TestUtils.h"

static void expectParseError(const std::string& source) {
    EXPECT_THROW(LangFactory().run(source), ParseError);
}
static void expectCheckError(const std::string& source) {
    EXPECT_THROW(LangFactory().run(source), CheckError);
}
static void expectRuntimeError(const std::string& source) {
    EXPECT_THROW(LangFactory().run(source), RuntimeError);
}

TEST(Ch2_Integration, BasicFunctionCallAndReturn) {
    EXPECT_EQ(execSource(
        "func add(a, b) { return a + b; }"
        "var ret = add(3, 7);"
        "print ret;"),
        "10\n");
}

TEST(Ch2_Integration, NoReturnReturnsNull) {
    EXPECT_EQ(execSource(
        "func noop() { var x = 1; }"
        "print noop();"),
        "null\n");
}

TEST(Ch2_Integration, RecursiveFactorial) {
    EXPECT_EQ(execSource(
        "func fact(n) {"
        "  if (n <= 1) return 1;"
        "  return n * fact(n - 1);"
        "}"
        "print fact(5);"),
        "120\n");
}

TEST(Ch2_Integration, FunctionWithClosure) {
    EXPECT_EQ(execSource(
        "var x = 10;"
        "func getX() { return x; }"
        "print getX();"),
        "10\n");
}

TEST(Ch2_Integration, FunctionMultipleParams) {
    EXPECT_EQ(execSource(
        "func mul(a, b, c) { return a * b * c; }"
        "print mul(2, 3, 4);"),
        "24\n");
}

TEST(Ch2_Integration, Error_ReturnOutsideFunction) {
    expectCheckError("return 5;");
}

TEST(Ch2_Integration, Error_DuplicateParam) {
    expectCheckError("func foo(a, a) { }");
}

TEST(Ch2_Integration, Error_CallNonCallable) {
    expectRuntimeError("var x = \"hello\"; x();");
}

TEST(Ch2_Integration, Error_ArityMismatch) {
    expectRuntimeError(
        "func foo(a, b, c) { }"
        "foo(1, 2);");
}

TEST(Ch3_Integration, ArrayCreateAndRead) {
    EXPECT_EQ(execSource(
        "var arr = Array(3);"
        "arr[0] = 10; arr[1] = 20; arr[2] = 30;"
        "print arr[0];"),
        "10\n");
}

TEST(Ch3_Integration, ArrayPrintMultiple) {
    EXPECT_EQ(execSource(
        "var arr = Array(3);"
        "arr[0] = 10; arr[1] = 20; arr[2] = 30;"
        "print arr[0]; print arr[1]; print arr[2];"),
        "10\n20\n30\n");
}

TEST(Ch3_Integration, ArrayDynamicIndex) {
    EXPECT_EQ(execSource(
        "var arr = Array(3);"
        "var i = 2;"
        "arr[i - 1] = 7;"
        "print arr[1];"),
        "7\n");
}

TEST(Ch3_Integration, ArrayInitialValueIsNull) {
    EXPECT_EQ(execSource(
        "var arr = Array(3);"
        "print arr[0];"),
        "null\n");
}

TEST(Ch3_Integration, Error_OutOfBounds) {
    expectRuntimeError(
        "var arr = Array(3);"
        "print arr[5];");
}

TEST(Ch3_Integration, Error_NonNumericIndex) {
    expectRuntimeError(
        "var arr = Array(3);"
        "print arr[\"hello\"];");
}

TEST(Ch3_Integration, Error_IndexOnNonArray) {
    expectRuntimeError("var x = 10; print x[0];");
}

TEST(Ch3_Integration, Error_NonNumericSize) {
    expectRuntimeError("var brr = Array(\"hi\");");
}

TEST(Ch3_Integration, Error_NegativeSize) {
    expectRuntimeError("var arr = Array(-1);");
}

TEST(Ch4_Integration, ConstantFolding_ArithResult) {
    EXPECT_EQ(execSource("print 1 + 2 * 3;"), "7\n");
}

TEST(Ch4_Integration, ConstantFolding_NestedExpression) {
    EXPECT_EQ(execSource("print 1 - 2 * 3 + 7 + 8 + 9;"), "19\n");
}

TEST(Ch4_Integration, StaticBinding_LocalVar) {
    EXPECT_EQ(execSource(
        "{"
        "  var a = 10;"
        "  print a;"
        "}"),
        "10\n");
}

TEST(Ch4_Integration, StaticBinding_NestedScope) {
    EXPECT_EQ(execSource(
        "var outer = 1;"
        "{"
        "  var inner = 2;"
        "  print outer + inner;"
        "}"),
        "3\n");
}

TEST(Ch4_Integration, StaticBinding_DeepNested) {
    EXPECT_EQ(execSource(
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

TEST(Integration_Combined, FunctionWithArray) {
    EXPECT_EQ(execSource(
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
    EXPECT_EQ(execSource(
        "func add(a, b) { return a + b; }"
        "print add(3 + 4, 2 * 5);"),
        "17\n");
}

TEST(Integration_Combined, ArrayInLoop) {
    EXPECT_EQ(execSource(
        "var arr = Array(3);"
        "for (var i = 0; i < 3; i = i + 1) {"
        "  arr[i] = i * 2;"
        "}"
        "print arr[0]; print arr[1]; print arr[2];"),
        "0\n2\n4\n");
}
