#include <gtest/gtest.h>
#include "Lexer.h"

TEST(LexerTest, EOFAlwaysAppended) {
    Lexer lexer;
    auto token_array = lexer.tokenize("");
    ASSERT_EQ((int)token_array.size(), 1);
    EXPECT_EQ(token_array[0].type, TokenType::END_OF_FILE);
}

TEST(LexerTest, LeftParenToken) {
    Lexer lexer;
    auto token_array = lexer.tokenize("(");
    EXPECT_EQ(token_array[0].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(token_array[0].lexeme, "(");
    EXPECT_EQ(token_array[0].line, 1);
    EXPECT_EQ((int)token_array.size(), 2);
}

TEST(LexerTest, AllSingleCharTokens) {
    Lexer lexer;
    auto token_array = lexer.tokenize("( ) { } ;");
    EXPECT_EQ(token_array[0].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(token_array[1].type, TokenType::RIGHT_PAREN);
    EXPECT_EQ(token_array[2].type, TokenType::LEFT_BRACE);
    EXPECT_EQ(token_array[3].type, TokenType::RIGHT_BRACE);
    EXPECT_EQ(token_array[4].type, TokenType::SEMICOLON);
    EXPECT_EQ(token_array[5].type, TokenType::END_OF_FILE);
    EXPECT_EQ((int)token_array.size(), 6);
}