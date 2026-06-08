#include <gtest/gtest.h>
#include <sstream>
#include "Debugger.h"
#include "TestUtils.h"

// 테스트용 소스 상수
static const std::string SIMPLE =
    "var a = 3;\n"      // line 1
    "var b = 7;\n"      // line 2
    "var result = a + b;\n"  // line 3
    "print result;\n";  // line 4

static const std::string LOOP_SOURCE =
    "var sum = 0;\n"                          // line 1
    "for (var i = 0; i < 3; i = i + 1) {\n"  // line 2
    "    sum = sum + i;\n"                    // line 3
    "}\n"
    "print sum;\n";                           // line 5

static const std::string ALL_TYPES =
    "var numVar  = 42;\n"
    "var strVar  = \"hello\";\n"
    "var boolVar = true;\n"
    "var arrVar  = Array(2);\n"
    "func fnVar() { return 1; }\n"
    "var nullVar;\n";

// ── Debugger 픽스처 ────────────────────────────────────────────────────

class DebuggerFixture : public ::testing::Test {
protected:
    std::string run(const std::string& source, const std::string& cmds) {
        std::istringstream cmdStream(cmds);
        Debugger dbg("virtual");
        return captureOutput([&]{ dbg.run(source, cmdStream); });
    }
};

// ── 기본 실행 모드 ─────────────────────────────────────────────────────

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

// ── Watch / Unwatch ────────────────────────────────────────────────────

TEST_F(DebuggerFixture, WatchAndUnwatch) {
    // line1: watch a, step
    // line2: auto-print [WATCH] a=3, watch b, step
    // line3: auto-print a b, watched, unwatch a, step
    // line4: auto-print b, step → 완료
    std::string out = run(SIMPLE,
        "watch a\nstep\n"
        "watch b\nstep\n"
        "watched\nunwatch a\nstep\n"
        "step\n");
    EXPECT_NE(out.find("WATCH"), std::string::npos);
}

TEST_F(DebuggerFixture, WatchedEmpty) {
    std::string out = run(SIMPLE, "watched\nexit\n");
    EXPECT_NE(out.find("없음"), std::string::npos);
}

TEST_F(DebuggerFixture, WatchUndefinedVar_ShowsMijeong) {
    // 미선언 변수 감시 → printWatches/cmdWatched catch 경로
    std::string out = run(SIMPLE, "watch undefinedVar\nwatched\nstep\nexit\n");
    EXPECT_NE(out.find("미정의"), std::string::npos);
}

// ── Breakpoint / Inspect ───────────────────────────────────────────────

TEST_F(DebuggerFixture, BreakpointAndInspect) {
    // line1: break 3, Breakpoints, continue
    // line2: m_stepMode=false → 건너뜀
    // line3: breakpoint 정지, inspect, remove 3, step
    // line4: step → 완료
    std::string out = run(SIMPLE,
        "break 3\nBreakpoints\ncontinue\n"
        "inspect\nremove 3\nstep\n"
        "step\n");
    EXPECT_NE(out.find("breakpoint"), std::string::npos);
}

TEST_F(DebuggerFixture, BreakpointsEmpty) {
    std::string out = run(SIMPLE, "Breakpoints\nexit\n");
    EXPECT_NE(out.find("없음"), std::string::npos);
}

// ── 커맨드 오류 처리 ───────────────────────────────────────────────────

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

// ── next: 블록 내부 건너뜀 ─────────────────────────────────────────────

TEST_F(DebuggerFixture, NextSkipsLoopBody) {
    // line1(var sum): step
    // line2(for):     next → m_nextDepth=1, 루프 내부(depth>1) 건너뜀
    // line5(print):   step → 완료, sum = 0+1+2 = 3 출력
    std::string out = run(LOOP_SOURCE, "step\nnext\nstep\n");
    EXPECT_NE(out.find("3"), std::string::npos);
}

// ── typeName 전 타입 경로 ──────────────────────────────────────────────

TEST_F(DebuggerFixture, Inspect_ShowsAllValueTypes) {
    // line 5(func 선언) 직전까지 실행 → lines 1-4 완료 후 inspect
    // break 5 + continue: line 5에서 정지, numVar/strVar/boolVar/arrVar 정의됨
    std::string out = run(ALL_TYPES, "break 5\ncontinue\ninspect\nexit\n");
    EXPECT_NE(out.find("Number"),  std::string::npos);
    EXPECT_NE(out.find("String"),  std::string::npos);
    EXPECT_NE(out.find("Boolean"), std::string::npos);
    EXPECT_NE(out.find("Array"),   std::string::npos);
}

// ── 에러 소스 처리 ─────────────────────────────────────────────────────

TEST_F(DebuggerFixture, ParseError_HandledGracefully) {
    EXPECT_NO_THROW(run("print 1 2;\n", ""));
}

TEST_F(DebuggerFixture, RuntimeError_HandledGracefully) {
    EXPECT_NO_THROW(run("print undeclaredVar;\n", ""));
}
