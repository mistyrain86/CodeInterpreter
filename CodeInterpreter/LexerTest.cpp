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