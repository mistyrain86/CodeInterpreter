#include <gtest/gtest.h>
#include <sstream>
#include "Shell.h"
#include "TestUtils.h"

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

TEST(ShellFromSource, PrintsOutput) {
    Shell shell;
    std::string out = captureOutput([&]{
        shell.runFromSource("print 42;", "test.cf");
    });
    EXPECT_NE(out.find("42"), std::string::npos);
}

TEST(ShellFromSource, PrintsStartMessages) {
    Shell shell;
    std::string out = captureOutput([&]{
        shell.runFromSource("print 1;", "myfile.cf");
    });
    EXPECT_NE(out.find("FILE"), std::string::npos);
    EXPECT_NE(out.find("myfile.cf"), std::string::npos);
}

TEST(ShellFromSource, MultiChunk_BothChunksRun) {
    Shell shell;
    std::string out = captureOutput([&]{
        shell.runFromSource("print 1;\n\nprint 2;\n", "test.cf");
    });
    EXPECT_NE(out.find("1"), std::string::npos);
    EXPECT_NE(out.find("2"), std::string::npos);
}

TEST(ShellFromSource, ParseError_StopsChunk) {
    Shell shell;
    EXPECT_NO_THROW(captureOutput([&]{
        shell.runFromSource("print 1 2;\n", "test.cf");
    }));
}

TEST(ShellFromSource, RuntimeError_StopsChunk) {
    Shell shell;
    EXPECT_NO_THROW(captureOutput([&]{
        shell.runFromSource("print undeclaredVar;\n", "test.cf");
    }));
}

TEST(ShellFromSource, CheckError_StopsChunk) {
    Shell shell;
    EXPECT_NO_THROW(captureOutput([&]{
        shell.runFromSource("{ var a = a; }\n", "test.cf");
    }));
}

TEST(ShellFromSource, LexerError_StopsChunk) {
    Shell shell;
    EXPECT_NO_THROW(captureOutput([&]{
        shell.runFromSource("@invalid;\n", "test.cf");
    }));
}

TEST(ShellFromSource, FunctionAndArray) {
    Shell shell;
    std::string out = captureOutput([&]{
        shell.runFromSource(
            "func add(a, b) { return a + b; }\n"
            "print add(3, 7);\n",
            "test.cf");
    });
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(ShellReplFixture, ExitCommand) {
    setInput("exit\n");
    std::string out = captureOutput([]{ Shell().runRepl(); });
    EXPECT_NE(out.find("REPL"), std::string::npos);
}

TEST_F(ShellReplFixture, QuitCommand) {
    setInput("quit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, EmptyLine_Skipped) {
    setInput("\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, ExecutesCode) {
    setInput("print 99;\nexit\n");
    std::string out = captureOutput([]{ Shell().runRepl(); });
    EXPECT_NE(out.find("99"), std::string::npos);
}

TEST_F(ShellReplFixture, StatePreservedAcrossLines) {
    setInput("var x = 10;\nprint x;\nexit\n");
    std::string out = captureOutput([]{ Shell().runRepl(); });
    EXPECT_NE(out.find("10"), std::string::npos);
}

TEST_F(ShellReplFixture, ParseError_ContinuesRepl) {
    setInput("print 1 2;\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, RuntimeError_ContinuesRepl) {
    setInput("print undeclaredVar;\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, CheckError_ContinuesRepl) {
    setInput("{ var a = a; }\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}

TEST_F(ShellReplFixture, LexerError_ContinuesRepl) {
    setInput("@invalid;\nexit\n");
    EXPECT_NO_THROW(captureOutput([]{ Shell().runRepl(); }));
}


