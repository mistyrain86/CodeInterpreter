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
    Lexer lexer;
    auto t = lexer.tokenize("\"hello\"");
    EXPECT_EQ(t[0].type, TokenType::STRING);
    EXPECT_EQ(std::get<std::string>(t[0].literal), "hello");
}

TEST(LexerTest, EmptyString) {
    Lexer lexer;
    EXPECT_EQ(std::get<std::string>(lexer.tokenize("\"\"")[0].literal), "");
}

TEST(LexerTest, UnterminatedString_Throws) {
    Lexer lexer;
    EXPECT_THROW(lexer.tokenize("\"hello"), std::runtime_error);
}

TEST(LexerTest, MultilineString_LineIncremented) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("\"he\nllo\"");
    EXPECT_EQ(tokenArray[0].type, TokenType::STRING);
    EXPECT_EQ(tokenArray[1].line, 2);
}

TEST(LexerTest, IntegerNumber) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("42");
    EXPECT_EQ(tokenArray[0].type, TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokenArray[0].literal), 42.0);
}

TEST(LexerTest, FloatNumber) {
    Lexer lexer;
    EXPECT_DOUBLE_EQ(std::get<double>(lexer.tokenize("3.14")[0].literal), 3.14);
}

TEST(LexerTest, ZeroNumber) {
    Lexer lexer;
    EXPECT_DOUBLE_EQ(std::get<double>(lexer.tokenize("0")[0].literal), 0.0);
}

TEST(LexerTest, Identifier) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("myVar");
    EXPECT_EQ(tokenArray[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[0].lexeme, "myVar");
}

TEST(LexerTest, IdentifierWithUnderscore) {
    Lexer lexer;
    EXPECT_EQ(lexer.tokenize("_count")[0].type, TokenType::IDENTIFIER);
}

TEST(LexerTest, Keyword_var) { Lexer lexer; EXPECT_EQ(lexer.tokenize("var")[0].type, TokenType::KW_VAR); }
TEST(LexerTest, Keyword_print) { Lexer lexer; EXPECT_EQ(lexer.tokenize("print")[0].type, TokenType::KW_PRINT); }
TEST(LexerTest, Keyword_if) { Lexer lexer; EXPECT_EQ(lexer.tokenize("if")[0].type, TokenType::KW_IF); }
TEST(LexerTest, Keyword_else) { Lexer lexer; EXPECT_EQ(lexer.tokenize("else")[0].type, TokenType::KW_ELSE); }
TEST(LexerTest, Keyword_for) { Lexer lexer; EXPECT_EQ(lexer.tokenize("for")[0].type, TokenType::KW_FOR); }
TEST(LexerTest, Keyword_true) { Lexer lexer; EXPECT_EQ(lexer.tokenize("true")[0].type, TokenType::KW_TRUE); }
TEST(LexerTest, Keyword_false) { Lexer lexer; EXPECT_EQ(lexer.tokenize("false")[0].type, TokenType::KW_FALSE); }

TEST(LexerTest, IdentifierNotKeyword) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("variable var");
    EXPECT_EQ(tokenArray[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[1].type, TokenType::KW_VAR);
}

TEST(LexerTest, LineNumberTracking) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("var\nif\nfor");
    EXPECT_EQ(tokenArray[0].line, 1);
    EXPECT_EQ(tokenArray[1].line, 2);
    EXPECT_EQ(tokenArray[2].line, 3);
}

TEST(LexerTest, WhitespaceIgnored) {
    Lexer lexer;
    auto tokenArray = lexer.tokenize("   +   -   ");
    EXPECT_EQ(tokenArray[0].type, TokenType::PLUS);
    EXPECT_EQ(tokenArray[1].type, TokenType::MINUS);
}

TEST(LexerTest, UnknownChar_At) { Lexer lexer; EXPECT_THROW(lexer.tokenize("@"), std::runtime_error); }
TEST(LexerTest, UnknownChar_Hash) { Lexer lexer; EXPECT_THROW(lexer.tokenize("#"), std::runtime_error); }
