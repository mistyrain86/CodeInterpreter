#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "Debugger.h"
#include "TestUtils.h"

static const char* DBG_TEMP = "._dbg_tmp.txt";
static void writeDbgTemp(const std::string& s) { std::ofstream f(DBG_TEMP); f << s; }
static void removeDbgTemp() { std::remove(DBG_TEMP); }

static const std::string SIMPLE =
    "var a = 3;\n"
    "var b = 7;\n"
    "var result = a + b;\n"
    "print result;\n";

static const std::string LOOP_SOURCE =
    "var sum = 0;\n"
    "for (var i = 0; i < 3; i = i + 1) {\n"
    "    sum = sum + i;\n"
    "}\n"
    "print sum;\n";

static const std::string ALL_TYPES =
    "var numVar  = 42;\n"
    "var strVar  = \"hello\";\n"
    "var boolVar = true;\n"
    "var arrVar  = Array(2);\n"
    "func fnVar() { return 1; }\n"
    "var nullVar;\n";

class DebuggerFixture : public ::testing::Test {
protected:
    std::string run(const std::string& source, const std::string& cmds) {
        std::istringstream cmdStream(cmds);
        Debugger dbg("virtual");
        return captureOutput([&]{ dbg.run(source, cmdStream); });
    }
};

TEST(DebuggerStandalone, InvalidFile_NoThrow) {
    Debugger dbg("nonexistent_xyz.cf");
    EXPECT_NO_THROW(captureOutput([&]{ dbg.run(); }));
}

TEST_F(DebuggerFixture, ExitImmediately) {
    std::string out = run(SIMPLE, "exit\n");
    EXPECT_NE(out.find("DEBUG"), std::string::npos);
}

TEST_F(DebuggerFixture, QuitCommand) {
    EXPECT_NO_THROW(run(SIMPLE, "quit\n"));
}

TEST_F(DebuggerFixture, StepThroughAll) {
    std::string out = run(SIMPLE, "step\nstep\nstep\nstep\nstep\n");
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(DebuggerFixture, Continue_SkipsAllStops) {
    std::string out = run(SIMPLE, "continue\n");
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(DebuggerFixture, WatchAndUnwatch) {
    std::string out = run(SIMPLE,
        "watch a\nstep\n"
        "watch b\nstep\n"
        "watched\nunwatch a\nstep\n"
        "step\n");
    EXPECT_NE(out.find("WATCH"), std::string::npos);
}

TEST_F(DebuggerFixture, WatchedEmpty) {
    std::string out = run(SIMPLE, "watched\nexit\n");
    EXPECT_NE(out.find("없습니다"), std::string::npos);
}

TEST_F(DebuggerFixture, WatchUndefinedVar_ShowsMijeong) {
    std::string out = run(SIMPLE, "watch undefinedVar\nwatched\nstep\nexit\n");
    EXPECT_NE(out.find("미정의"), std::string::npos);
}

TEST_F(DebuggerFixture, BreakpointAndInspect) {
    std::string out = run(SIMPLE,
        "break 3\nBreakpoints\ncontinue\n"
        "inspect\nremove 3\nstep\n"
        "step\n");
    EXPECT_NE(out.find("breakpoint"), std::string::npos);
}

TEST_F(DebuggerFixture, BreakpointsEmpty) {
    std::string out = run(SIMPLE, "Breakpoints\nexit\n");
    EXPECT_NE(out.find("없습니다"), std::string::npos);
}

TEST_F(DebuggerFixture, UnknownCommand_ShowsHelp) {
    std::string out = run(SIMPLE, "unknownXYZ\nexit\n");
    EXPECT_NE(out.find("step"), std::string::npos);
}

TEST_F(DebuggerFixture, InvalidBreakArg_ShowsUsage) {
    std::string out = run(SIMPLE, "break abc\nexit\n");
    EXPECT_NE(out.find("줄번호"), std::string::npos);
}

TEST_F(DebuggerFixture, InvalidRemoveArg_ShowsUsage) {
    std::string out = run(SIMPLE, "remove abc\nexit\n");
    EXPECT_NE(out.find("줄번호"), std::string::npos);
}

TEST_F(DebuggerFixture, NextSkipsLoopBody) {
    std::string out = run(LOOP_SOURCE, "step\nnext\nstep\n");
    EXPECT_NE(out.find("3"), std::string::npos);
}

TEST_F(DebuggerFixture, Inspect_ShowsAllValueTypes) {
    // break 6: lines 1-5 실행 후 정지 → fnVar(Function) 포함 모든 타입 확인
    std::string out = run(ALL_TYPES, "break 6\ncontinue\ninspect\nexit\n");
    EXPECT_NE(out.find("Number"),   std::string::npos);
    EXPECT_NE(out.find("String"),   std::string::npos);
    EXPECT_NE(out.find("Boolean"),  std::string::npos);
    EXPECT_NE(out.find("Array"),    std::string::npos);
    EXPECT_NE(out.find("Function"), std::string::npos);
}

TEST_F(DebuggerFixture, ParseError_HandledGracefully) {
    EXPECT_NO_THROW(run("print 1 2;\n", ""));
}

TEST_F(DebuggerFixture, RuntimeError_HandledGracefully) {
    // var x = 1; x() → RuntimeError catch 경로
    EXPECT_NO_THROW(run("var x = 1;\nx();\n", "step\nstep\n"));
}

TEST_F(DebuggerFixture, LexerError_HandledGracefully) {
    // @ → std::runtime_error catch 경로 (Lexer에서 throw)
    EXPECT_NO_THROW(run("@invalid;\n", ""));
}

TEST(DebuggerFileTest, Run_NoArg_ValidFile) {
    writeDbgTemp("print 7;\n");
    std::istringstream cmds("exit\n");
    auto* old = std::cin.rdbuf(cmds.rdbuf());
    std::string out = captureOutput([]{
        Debugger dbg(DBG_TEMP);
        dbg.run();
    });
    std::cin.rdbuf(old);
    removeDbgTemp();
    EXPECT_NE(out.find("DEBUG"), std::string::npos);
}
