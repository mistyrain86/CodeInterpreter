#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "Shell.h"
#include "TestUtils.h"

static const std::string SHELL_CF  = "._shell_test.cf";
static const std::string SHELL_CF2 = "._shell_test2.cf";

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
}

class ShellFileFixture : public ::testing::Test {
protected:
    void SetUp()    override { writeFile(SHELL_CF, "print 42;\n"); }
    void TearDown() override { std::remove(SHELL_CF.c_str()); }
};

class ShellReplFixture : public ::testing::Test {
protected:
    std::istringstream m_input;
    std::streambuf*    m_oldCin = nullptr;

    void setInput(const std::string& s) {
        m_input.str(s); m_input.clear();
        m_oldCin = std::cin.rdbuf(m_input.rdbuf());
    }
    void TearDown() override {
        if (m_oldCin) std::cin.rdbuf(m_oldCin);
    }
};

TEST_F(ShellFileFixture, RunFile_PrintsOutput) {
    Shell shell;
    std::string out = captureOutput([&]{ shell.runFile(SHELL_CF); });
    EXPECT_NE(out.find("42"), std::string::npos);
}

TEST_F(ShellFileFixture, RunFile_PrintsStartMessages) {
    Shell shell;
    std::string out = captureOutput([&]{ shell.runFile(SHELL_CF); });
    EXPECT_NE(out.find("FILE"), std::string::npos);
}

TEST(ShellFileTest, RunFile_MultiChunk_BothChunksRun) {
    writeFile(SHELL_CF2, "print 1;\n\nprint 2;\n");
    Shell shell;
    std::string out = captureOutput([&]{ shell.runFile(SHELL_CF2); });
    EXPECT_NE(out.find("1"), std::string::npos);
    EXPECT_NE(out.find("2"), std::string::npos);
    std::remove(SHELL_CF2.c_str());
}

TEST(ShellFileTest, RunFile_ErrorChunk_ContinuesGracefully) {
    writeFile(SHELL_CF2, "print undeclaredVar;\n");
    Shell shell;
    EXPECT_NO_THROW(captureOutput([&]{ shell.runFile(SHELL_CF2); }));
    std::remove(SHELL_CF2.c_str());
}

TEST_F(ShellReplFixture, RunRepl_ExitCommand) {
    setInput("exit\n");
    std::string out = captureOutput([]{ Shell().runRepl(); });
    EXPECT_NE(out.find("REPL"), std::string::npos);
}

TEST_F(ShellReplFixture, RunRepl_QuitCommand) {
    setInput("quit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, RunRepl_EmptyLine_Skipped) {
    setInput("\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, RunRepl_ExecutesCode) {
    setInput("print 99;\nexit\n");
    std::string out = captureOutput([]{ Shell().runRepl(); });
    EXPECT_NE(out.find("99"), std::string::npos);
}

TEST_F(ShellReplFixture, RunRepl_StatePreservedAcrossLines) {
    setInput("var x = 10;\nprint x;\nexit\n");
    std::string out = captureOutput([]{ Shell().runRepl(); });
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(ShellReplFixture, RunRepl_ParseError_ContinuesRepl) {
    setInput("print 1 2;\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, RunRepl_RuntimeError_ContinuesRepl) {
    setInput("print undeclaredVar;\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, RunRepl_CheckError_ContinuesRepl) {
    setInput("{ var a = a; }\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, RunRepl_LexerError_ContinuesRepl) {
    setInput("@invalid;\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST(ShellDebugTest, RunDebug_ExitImmediately) {
    writeFile(SHELL_CF2, "print 1;\n");
    std::istringstream input("exit\n");
    auto* oldCin = std::cin.rdbuf(input.rdbuf());
    EXPECT_NO_THROW(captureOutput([]{
        Shell().runDebug("._shell_test2.cf");
    }));
    std::cin.rdbuf(oldCin);
    std::remove(SHELL_CF2.c_str());
}
