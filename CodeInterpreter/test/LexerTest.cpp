#include <gtest/gtest.h>
#include "Lexer.h"

class LexerFixture : public ::testing::Test {
protected:
    Lexer lexer;
};

TEST_F(LexerFixture, EOFAlwaysAppended) {
    auto tokenArray = lexer.tokenize("");
    ASSERT_EQ((int)tokenArray.size(), 1);
    EXPECT_EQ(tokenArray[0].type, TokenType::END_OF_FILE);
}

TEST_F(LexerFixture, LeftParenToken) {
    auto tokenArray = lexer.tokenize("(");
    EXPECT_EQ(tokenArray[0].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokenArray[0].lexeme, "(");
    EXPECT_EQ(tokenArray[0].line, 1);
    EXPECT_EQ((int)tokenArray.size(), 2);
}

TEST_F(LexerFixture, AllSingleCharTokens) {
    auto tokenArray = lexer.tokenize("( ) { } ;");
    EXPECT_EQ(tokenArray[0].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokenArray[1].type, TokenType::RIGHT_PAREN);
    EXPECT_EQ(tokenArray[2].type, TokenType::LEFT_BRACE);
    EXPECT_EQ(tokenArray[3].type, TokenType::RIGHT_BRACE);
    EXPECT_EQ(tokenArray[4].type, TokenType::SEMICOLON);
    EXPECT_EQ(tokenArray[5].type, TokenType::END_OF_FILE);
    EXPECT_EQ((int)tokenArray.size(), 6);
}

TEST_F(LexerFixture, ArithmeticOperators) {
    auto tokenArray = lexer.tokenize("+ - * /");
    EXPECT_EQ(tokenArray[0].type, TokenType::PLUS);
    EXPECT_EQ(tokenArray[1].type, TokenType::MINUS);
    EXPECT_EQ(tokenArray[2].type, TokenType::STAR);
    EXPECT_EQ(tokenArray[3].type, TokenType::SLASH);
}

TEST_F(LexerFixture, LogicalAndAssignmentOperators) {
    auto tokenArray = lexer.tokenize("= !");
    EXPECT_EQ(tokenArray[0].type, TokenType::EQUAL);
    EXPECT_EQ(tokenArray[1].type, TokenType::BANG);
}

TEST_F(LexerFixture, ComparisonOperators) {
    auto tokenArray = lexer.tokenize("== != <= >=");
    EXPECT_EQ(tokenArray[0].type, TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokenArray[1].type, TokenType::BANG_EQUAL);
    EXPECT_EQ(tokenArray[2].type, TokenType::LESS_EQUAL);
    EXPECT_EQ(tokenArray[3].type, TokenType::GREATER_EQUAL);
}

TEST_F(LexerFixture, SingleVsCompound) {
    auto tokenArray = lexer.tokenize("= == ! !=");
    EXPECT_EQ(tokenArray[0].type, TokenType::EQUAL);
    EXPECT_EQ(tokenArray[1].type, TokenType::EQUAL_EQUAL);
    EXPECT_EQ(tokenArray[2].type, TokenType::BANG);
    EXPECT_EQ(tokenArray[3].type, TokenType::BANG_EQUAL);
}

TEST_F(LexerFixture, LineCommentIgnored) {
    auto tokenArray = lexer.tokenize("+ // 주석\n-");
    EXPECT_EQ(tokenArray[0].type, TokenType::PLUS);
    EXPECT_EQ(tokenArray[1].type, TokenType::MINUS);
    EXPECT_EQ((int)tokenArray.size(), 3);
}

TEST_F(LexerFixture, CommentOnly) {
    EXPECT_EQ((int)lexer.tokenize("// 전체 주석").size(), 1);
}

TEST_F(LexerFixture, StringLiteral) {
    auto tokenArray = lexer.tokenize("\"hello\"");
    EXPECT_EQ(tokenArray[0].type, TokenType::STRING);
    EXPECT_EQ(std::get<std::string>(tokenArray[0].literal), "hello");
}

TEST_F(LexerFixture, EmptyString) {
    EXPECT_EQ(std::get<std::string>(lexer.tokenize("\"\"")[0].literal), "");
}

TEST_F(LexerFixture, UnterminatedString_Throws) {
    EXPECT_THROW(lexer.tokenize("\"hello"), std::runtime_error);
}

TEST_F(LexerFixture, MultilineString_LineIncremented) {
    auto tokenArray = lexer.tokenize("\"he\nllo\"");
    EXPECT_EQ(tokenArray[0].type, TokenType::STRING);
    EXPECT_EQ(tokenArray[1].line, 2);
}

TEST_F(LexerFixture, IntegerNumber) {
    auto tokenArray = lexer.tokenize("42");
    EXPECT_EQ(tokenArray[0].type, TokenType::NUMBER);
    EXPECT_DOUBLE_EQ(std::get<double>(tokenArray[0].literal), 42.0);
}

TEST_F(LexerFixture, FloatNumber) {
    EXPECT_DOUBLE_EQ(std::get<double>(lexer.tokenize("3.14")[0].literal), 3.14);
}

TEST_F(LexerFixture, ZeroNumber) {
    EXPECT_DOUBLE_EQ(std::get<double>(lexer.tokenize("0")[0].literal), 0.0);
}

TEST_F(LexerFixture, Identifier) {
    auto tokenArray = lexer.tokenize("myVar");
    EXPECT_EQ(tokenArray[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[0].lexeme, "myVar");
}

TEST_F(LexerFixture, IdentifierWithUnderscore) {
    EXPECT_EQ(lexer.tokenize("_count")[0].type, TokenType::IDENTIFIER);
}

TEST_F(LexerFixture, Keyword_var)   { EXPECT_EQ(lexer.tokenize("var")[0].type,   TokenType::KW_VAR);   }
TEST_F(LexerFixture, Keyword_print) { EXPECT_EQ(lexer.tokenize("print")[0].type, TokenType::KW_PRINT); }
TEST_F(LexerFixture, Keyword_if)    { EXPECT_EQ(lexer.tokenize("if")[0].type,    TokenType::KW_IF);    }
TEST_F(LexerFixture, Keyword_else)  { EXPECT_EQ(lexer.tokenize("else")[0].type,  TokenType::KW_ELSE);  }
TEST_F(LexerFixture, Keyword_for)   { EXPECT_EQ(lexer.tokenize("for")[0].type,   TokenType::KW_FOR);   }
TEST_F(LexerFixture, Keyword_true)  { EXPECT_EQ(lexer.tokenize("true")[0].type,  TokenType::KW_TRUE);  }
TEST_F(LexerFixture, Keyword_false) { EXPECT_EQ(lexer.tokenize("false")[0].type, TokenType::KW_FALSE); }

TEST_F(LexerFixture, IdentifierNotKeyword) {
    auto tokenArray = lexer.tokenize("variable var");
    EXPECT_EQ(tokenArray[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[1].type, TokenType::KW_VAR);
}

TEST_F(LexerFixture, LineNumberTracking) {
    auto tokenArray = lexer.tokenize("var\nif\nfor");
    EXPECT_EQ(tokenArray[0].line, 1);
    EXPECT_EQ(tokenArray[1].line, 2);
    EXPECT_EQ(tokenArray[2].line, 3);
}

TEST_F(LexerFixture, WhitespaceIgnored) {
    auto tokenArray = lexer.tokenize("   +   -   ");
    EXPECT_EQ(tokenArray[0].type, TokenType::PLUS);
    EXPECT_EQ(tokenArray[1].type, TokenType::MINUS);
}

TEST_F(LexerFixture, UnknownChar_At)   { EXPECT_THROW(lexer.tokenize("@"), std::runtime_error); }
TEST_F(LexerFixture, UnknownChar_Hash) { EXPECT_THROW(lexer.tokenize("#"), std::runtime_error); }

TEST_F(LexerFixture, FuncKeyword) {
    auto tokenArray = lexer.tokenize("func");
    EXPECT_EQ(tokenArray[0].type, TokenType::KW_FUNC);
}

TEST_F(LexerFixture, ReturnKeyword) {
    auto tokenArray = lexer.tokenize("return");
    EXPECT_EQ(tokenArray[0].type, TokenType::KW_RETURN);
}

TEST_F(LexerFixture, Comma) {
    auto tokenArray = lexer.tokenize(",");
    EXPECT_EQ(tokenArray[0].type, TokenType::COMMA);
}

TEST_F(LexerFixture, FuncSignature) {
    auto tokenArray = lexer.tokenize("func add(a, b)");
    EXPECT_EQ(tokenArray[0].type, TokenType::KW_FUNC);
    EXPECT_EQ(tokenArray[1].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[2].type, TokenType::LEFT_PAREN);
    EXPECT_EQ(tokenArray[3].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[4].type, TokenType::COMMA);
    EXPECT_EQ(tokenArray[5].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[6].type, TokenType::RIGHT_PAREN);
}

TEST_F(LexerFixture, LeftBracket) {
    auto tokenArray = lexer.tokenize("[");
    EXPECT_EQ(tokenArray[0].type, TokenType::LEFT_BRACKET);
}

TEST_F(LexerFixture, RightBracket) {
    auto tokenArray = lexer.tokenize("]");
    EXPECT_EQ(tokenArray[0].type, TokenType::RIGHT_BRACKET);
}

TEST_F(LexerFixture, ArrayAccess) {
    auto tokenArray = lexer.tokenize("arr[0]");
    EXPECT_EQ(tokenArray[0].type, TokenType::IDENTIFIER);
    EXPECT_EQ(tokenArray[1].type, TokenType::LEFT_BRACKET);
    EXPECT_EQ(tokenArray[2].type, TokenType::NUMBER);
    EXPECT_EQ(tokenArray[3].type, TokenType::RIGHT_BRACKET);
    EXPECT_DOUBLE_EQ(std::get<double>(tokenArray[2].literal), 0.0);
}

TEST_F(LexerFixture, CarriageReturn_Ignored) {
    auto tokenArray = lexer.tokenize("var\r\na = 1;");
    EXPECT_EQ(tokenArray[0].type, TokenType::KW_VAR);
    EXPECT_EQ(tokenArray[1].type, TokenType::IDENTIFIER);
}

