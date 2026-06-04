#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "LangFactory.h"
#include "ParseError.h"
#include "CheckError.h"
#include "Parser.h"
#include "TestUtils.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::Throw;

// 토큰 시퀀스 헬퍼
static Token tok(TokenType t, std::string lex, int line = 1) {
    return Token{t, std::move(lex), std::monostate{}, line};
}
static Token numTok(double v, int line = 1) {
    return Token{TokenType::NUMBER, std::to_string(v), v, line};
}
static Token eofTok() {
    return Token{TokenType::END_OF_FILE, "", std::monostate{}, 1};
}

static auto emptyParse() {
    return ::testing::InvokeWithoutArgs([]() -> std::vector<StmtPtr> { return {}; });
}

class LangFactoryFixture : public ::testing::Test {
protected:
    MockLexer*       ml = nullptr;
    MockParser*      mp = nullptr;
    MockChecker*     mc = nullptr;
    MockInterpreter* mi = nullptr;
    std::unique_ptr<LangFactory> factory;

    void SetUp() override {
        auto lexer       = std::make_unique<MockLexer>();
        auto parser      = std::make_unique<MockParser>();
        auto checker     = std::make_unique<MockChecker>();
        auto interpreter = std::make_unique<MockInterpreter>();

        ml = lexer.get();
        mp = parser.get();
        mc = checker.get();
        mi = interpreter.get();

        factory = std::make_unique<LangFactory>(
            std::move(lexer), std::move(parser),
            std::move(checker), std::move(interpreter));
    }
};

TEST_F(LangFactoryFixture, Pipeline_CallsInOrder) {
    InSequence seq;
    EXPECT_CALL(*ml, tokenize("print 5;")).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(emptyParse());
    EXPECT_CALL(*mc, check(_)).Times(1);
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    factory->run("print 5;");
}

TEST_F(LangFactoryFixture, ParseError_StopsBeforeChecker) {
    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(Throw(ParseError("[라인 1] 구문 오류: 테스트")));
    EXPECT_CALL(*mc, check(_)).Times(0);
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), ParseError);
}

TEST_F(LangFactoryFixture, CheckerError_StopsBeforeInterpreter) {
    EXPECT_CALL(*ml, tokenize(_)).Times(1);
    EXPECT_CALL(*mp, parse(_))
        .WillOnce(emptyParse());
    EXPECT_CALL(*mc, check(_))
        .WillOnce(Throw(CheckError("[라인 1] 의미 오류: 테스트")));
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), CheckError);
}

// ── Real Parser 통합 픽스처 ──────────────────────────────────────
class RealParserFixture : public ::testing::Test {
protected:
    MockLexer*       ml = nullptr;
    MockChecker*     mc = nullptr;
    MockInterpreter* mi = nullptr;
    std::unique_ptr<LangFactory> factory;

    void SetUp() override {
        auto lexer       = std::make_unique<MockLexer>();
        auto checker     = std::make_unique<MockChecker>();
        auto interpreter = std::make_unique<MockInterpreter>();

        ml = lexer.get();
        mc = checker.get();
        mi = interpreter.get();

        factory = std::make_unique<LangFactory>(
            std::move(lexer),
            std::make_unique<Parser>(),
            std::move(checker),
            std::move(interpreter));
    }
};

TEST_F(RealParserFixture, PrintStmt_Integration) {
    EXPECT_CALL(*ml, tokenize(_)).WillOnce(::testing::Return(std::vector<Token>{
        tok(TokenType::KW_PRINT, "print"),
        numTok(5.0),
        tok(TokenType::SEMICOLON, ";"),
        eofTok()
    }));
    EXPECT_CALL(*mc, check(_)).Times(1);
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    factory->run("print 5;");
}

TEST_F(RealParserFixture, VarDecl_Integration) {
    EXPECT_CALL(*ml, tokenize(_)).WillOnce(::testing::Return(std::vector<Token>{
        tok(TokenType::KW_VAR,    "var"),
        tok(TokenType::IDENTIFIER,"x"),
        tok(TokenType::EQUAL,     "="),
        numTok(10.0),
        tok(TokenType::SEMICOLON, ";"),
        eofTok()
    }));
    EXPECT_CALL(*mc, check(_)).Times(1);
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    factory->run("var x = 10;");
}

TEST_F(RealParserFixture, ParseError_RealParser_Propagates) {
    EXPECT_CALL(*ml, tokenize(_)).WillOnce(::testing::Return(std::vector<Token>{
        numTok(1.0),
        numTok(2.0),
        eofTok()
    }));
    EXPECT_CALL(*mc, check(_)).Times(0);
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    EXPECT_THROW(factory->run(""), ParseError);
}
