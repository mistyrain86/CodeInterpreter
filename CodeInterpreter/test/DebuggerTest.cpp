#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "Debugger.h"
#include "TestUtils.h"

static const std::string DBG_SIMPLE = "._dbg_simple.cf";
static const std::string DBG_LOOP   = "._dbg_loop.cf";
static const std::string DBG_TYPES  = "._dbg_types.cf";

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
}

class DebuggerFixture : public ::testing::Test {
protected:
    std::istringstream m_input;
    std::streambuf*    m_oldCin = nullptr;

    void SetUp() override {
        writeFile(DBG_SIMPLE,
            "var a = 3;\n"
            "var b = 7;\n"
            "var result = a + b;\n"
            "print result;\n");
    }

    void setInput(const std::string& s) {
        m_input.str(s); m_input.clear();
        m_oldCin = std::cin.rdbuf(m_input.rdbuf());
    }

    void TearDown() override {
        if (m_oldCin) std::cin.rdbuf(m_oldCin);
        std::remove(DBG_SIMPLE.c_str());
    }
};

TEST(DebuggerStandalone, Run_InvalidFile_NoThrow) {
    Debugger dbg("nonexistent_file_xyz.cf");
    EXPECT_NO_THROW(captureOutput([&]{ dbg.run(); }));
}

TEST_F(DebuggerFixture, Run_ExitImmediately) {
    setInput("exit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("DEBUG"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_QuitCommand) {
    setInput("quit\n");
    Debugger dbg(DBG_SIMPLE);
    EXPECT_NO_THROW(captureOutput([&]{ dbg.run(); }));
}

TEST_F(DebuggerFixture, Run_StepThroughAll) {
    setInput("step\nstep\nstep\nstep\nstep\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_Continue_SkipsAllStops) {
    setInput("continue\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_WatchAndUnwatch) {
    // line1: watch a, step
    // line2: auto-print a, watch b, step
    // line3: auto-print a b, watched, unwatch a, step
    // line4: auto-print b, step → done
    setInput("watch a\nstep\nwatch b\nstep\nwatched\nunwatch a\nstep\nstep\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("WATCH"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_WatchUndefined_ShowsMijeong) {
    // watch 미선언 변수 → printWatches/cmdWatched catch 경로
    setInput("watch undefinedVar\nwatched\nstep\nexit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("미정의"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_WatchedEmpty) {
    setInput("watched\nexit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("없음"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_BreakpointAndInspect) {
    // line1: break 3, Breakpoints, continue
    // line2: m_stepMode=false → skipped
    // line3: breakpoint → stop, inspect, remove 3, step
    // line4: step → done
    setInput("break 3\nBreakpoints\ncontinue\ninspect\nremove 3\nstep\nstep\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("breakpoint"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_BreakpointsEmpty) {
    setInput("Breakpoints\nexit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("없음"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_UnknownCommand_ShowsHelp) {
    setInput("unknownXYZ\nexit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("step"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_InvalidBreakArg_ShowsUsage) {
    setInput("break abc\nexit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("줄번호"), std::string::npos);
}

TEST_F(DebuggerFixture, Run_InvalidRemoveArg_ShowsUsage) {
    setInput("remove abc\nexit\n");
    Debugger dbg(DBG_SIMPLE);
    std::string out = captureOutput([&]{ dbg.run(); });
    EXPECT_NE(out.find("줄번호"), std::string::npos);
}

TEST(DebuggerLoop, Run_NextSkipsLoopBody) {
    writeFile(DBG_LOOP,
        "var sum = 0;\n"
        "for (var i = 0; i < 3; i = i + 1) {\n"
        "    sum = sum + i;\n"
        "}\n"
        "print sum;\n");

    std::istringstream input("step\nnext\nstep\n");
    auto* old = std::cin.rdbuf(input.rdbuf());

    Debugger dbg(DBG_LOOP);
    std::string out = captureOutput([&]{ dbg.run(); });
    std::cin.rdbuf(old);
    std::remove(DBG_LOOP.c_str());

    EXPECT_NE(out.find("3"), std::string::npos);
}

TEST(DebuggerTypes, Run_Inspect_ShowsAllTypes) {
    writeFile(DBG_TYPES,
        "var numVar  = 42;\n"
        "var strVar  = \"hello\";\n"
        "var boolVar = true;\n"
        "var arrVar  = Array(2);\n"
        "func fnVar() { return 1; }\n"
        "var nullVar;\n");

    std::istringstream input(
        "step\nstep\nstep\nstep\nstep\nstep\n"
        "inspect\nexit\n");
    auto* old = std::cin.rdbuf(input.rdbuf());

    Debugger dbg(DBG_TYPES);
    std::string out = captureOutput([&]{ dbg.run(); });
    std::cin.rdbuf(old);
    std::remove(DBG_TYPES.c_str());

    EXPECT_NE(out.find("Number"),  std::string::npos);
    EXPECT_NE(out.find("String"),  std::string::npos);
    EXPECT_NE(out.find("Boolean"), std::string::npos);
    EXPECT_NE(out.find("Array"),   std::string::npos);
}

TEST(DebuggerError, Run_ParseError_HandledGracefully) {
    static const std::string errFile = "._dbg_err.cf";
    writeFile(errFile, "print 1 2;\n");
    std::istringstream input("");
    auto* old = std::cin.rdbuf(input.rdbuf());
    EXPECT_NO_THROW(captureOutput([&]{
        Debugger dbg(errFile);
        dbg.run();
    }));
    std::cin.rdbuf(old);
    std::remove(errFile.c_str());
}
