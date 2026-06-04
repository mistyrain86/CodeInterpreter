#include <gtest/gtest.h>
#include "Lexer.h"

TEST(LexerTest, EOFAlwaysAppended) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("");
    ASSERT_EQ((int)tokenArray.size(), 1);
    EXPECT_EQ(tokenArray[0].type, TokenType::END_OF_FILE);
}

TEST(LexerTest, LeftParenToken) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("(");
    EXPECT_EQ(tokenArray[0].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokenArray[0].lexeme, "(");
    EXPECT_EQ(tokenArray[0].line, 1);
    EXPECT_EQ((int)tokenArray.size(), 2);
}

TEST(LexerTest, AllSingleCharTokens) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("( ) { } ;");
    EXPECT_EQ(tokenArray[0].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokenArray[1].type, TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokenArray[2].type, TokenType::LEFT_BRACE);
    EXPECT_EQ(tokenArray[3].type, TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokenArray[4].type, TokenType::SEMICOLON);
    EXPECT_EQ(tokenArray[5].type, TokenType::END_OF_FILE);
    EXPECT_EQ((int)tokenArray.size(), 6);
}

TEST(LexerTest, ArithmeticOperators) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("+ - * /");
    EXPECT_EQ(tokenArray[0].type, TokenType::PLUS);
    EXPECT_EQ(tokenArray[1].type, TokenType::MINUS);
    EXPECT_EQ(tokenArray[2].type, TokenType::STAR);
    EXPECT_EQ(tokenArray[3].type, TokenType::SLASH);
}

TEST(LexerTest, LogicalAndAssignmentOperators) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("= !");

    EXPECT_EQ(tokenArray[0].type, TokenType::EQUAL);
    EXPECT_EQ(tokenArray[1].type, TokenType::BANG);
}

TEST(LexerTest, ComparisonOperators) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("== != <= >=");

    EXPECT_EQ(tokenArray[0].type, TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokenArray[1].type, TokenType::BANG_EQUAL);
    EXPECT_EQ(tokenArray[2].type, TokenType::LESS_EQUAL);
    EXPECT_EQ(tokenArray[3].type, TokenType::GREATER_EQUAL);
}

TEST(LexerTest, SingleVsCompound) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("= == ! !=");
    EXPECT_EQ(tokenArray[0].type, TokenType::EQUAL);
    EXPECT_EQ(tokenArray[1].type, TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokenArray[2].type, TokenType::BANG);
    EXPECT_EQ(tokenArray[3].type, TokenType::BANG_EQUAL);
}

TEST(LexerTest, LineCommentIgnored) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("+ // 주석\n-");
    EXPECT_EQ(tokenArray[0].type, TokenType::PLUS);
    EXPECT_EQ(tokenArray[1].type, TokenType::MINUS);
    EXPECT_EQ((int)tokenArray.size(), 3);
}

TEST(LexerTest, CommentOnly) {
    Lexer lexer;
    EXPECT_EQ((int)lexer.tokenize("// 전체 주석").size(), 1);
}

TEST(LexerTest, StringLiteral) {
    Lexer l;
    auto t = l.tokenize("\"hello\"");
    EXPECT_EQ(t[0].type, TokenType::STRING);
    EXPECT_EQ(std::get<std::string>(t[0].literal), "hello");
}

TEST(LexerTest, EmptyString) {
    Lexer l;
    EXPECT_EQ(std::get<std::string>(l.tokenize("\"\"")[0].literal), "");
}

TEST(LexerTest, UnterminatedString_Throws) {
    Lexer l;
    EXPECT_THROW(l.tokenize("\"hello"), std::runtime_error);
}

TEST(LexerTest, IntegerNumber) {
    Lexer l;
    auto t = l.tokenize("42");
    EXPECT_EQ(t[0].type, TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(t[0].literal), 42.0);
}

TEST(LexerTest, FloatNumber) {
    Lexer l;
    EXPECT_DOUBLE_EQ(std::get<double>(l.tokenize("3.14")[0].literal), 3.14);
}

TEST(LexerTest, ZeroNumber) {
    Lexer l;
    EXPECT_DOUBLE_EQ(std::get<double>(l.tokenize("0")[0].literal), 0.0);
}