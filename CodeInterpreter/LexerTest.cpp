#include <gtest/gtest.h>
#include "Lexer.h"

TEST(LexerTest, EOFAlwaysAppended) {
    Lexer lexer;
    auto token_array = lexer.tokenize("");
    ASSERT_EQ((int)token_array.size(), 1);
    EXPECT_EQ(token_array[0].type, TokenType::END_OF_FILE);
}
