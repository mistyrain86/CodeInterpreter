#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "Lexer.h"
#include "Parser.h"
#include "LangFactory.h"
#include "TestUtils.h"

using ::testing::_;
using ::testing::Return;

// ── 단위 테스트 헬퍼 ──────────────────────────────────────
static Token t(TokenType type, std::string lex,
               std::variant<std::monostate,double,std::string> lit = std::monostate{},
               int line = 1) {
    return Token{type, std::move(lex), std::move(lit), line};
}
static Token eof()  { return t(TokenType::END_OF_FILE, ""); }
static Token semi() { return t(TokenType::SEMICOLON, ";"); }

static std::vector<StmtPtr> parse(std::vector<Token> tokens) {
    return Parser().parse(std::move(tokens));
}

// ── 단위 테스트 ───────────────────────────────────────────
TEST(ParserUnit, NumberLiteral) {
    auto stmts = parse({t(TokenType::NUMBER,"5",5.0), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 5.0);
}
TEST(ParserUnit, StringLiteral) {
    auto stmts = parse({t(TokenType::STRING,"\"hi\"",std::string("hi")), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<std::string>(lit->value), "hi");
}
TEST(ParserUnit, BoolTrue) {
    auto stmts = parse({t(TokenType::KW_TRUE,"true"), semi(), eof()});
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<bool>(lit->value), true);
}
TEST(ParserUnit, BoolFalse) {
    auto stmts = parse({t(TokenType::KW_FALSE,"false"), semi(), eof()});
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<bool>(lit->value), false);
}
TEST(ParserUnit, Grouping) {
    auto stmts = parse({t(TokenType::LEFT_PAREN,"("),
                        t(TokenType::NUMBER,"5",5.0),
                        t(TokenType::RIGHT_PAREN,")"), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<GroupingExpr*>(es->expression.get()), nullptr);
}

TEST(ParserUnit, UnaryMinus) {
    auto stmts = parse({t(TokenType::MINUS,"-"),
                        t(TokenType::NUMBER,"3",3.0), semi(), eof()});
    auto* un = dynamic_cast<UnaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->expression.get());
    ASSERT_NE(un, nullptr);
    EXPECT_EQ(un->op.type, TokenType::MINUS);
}
TEST(ParserUnit, UnaryBang) {
    auto stmts = parse({t(TokenType::BANG,"!"),
                        t(TokenType::KW_TRUE,"true"), semi(), eof()});
    auto* un = dynamic_cast<UnaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->expression.get());
    ASSERT_NE(un, nullptr);
    EXPECT_EQ(un->op.type, TokenType::BANG);
}

TEST(ParserUnit, MissingSemicolon_Throws) {
    EXPECT_THROW(parse({t(TokenType::KW_PRINT,"print"),
                        t(TokenType::NUMBER,"5",5.0), eof()}), ParseError);
}
TEST(ParserUnit, MissingCloseParen_Throws) {
    EXPECT_THROW(parse({t(TokenType::KW_PRINT,"print"),
                        t(TokenType::LEFT_PAREN,"("),
                        t(TokenType::NUMBER,"1",1.0), semi(), eof()}), ParseError);
}
TEST(ParserUnit, ExpectExpression_Throws) {
    EXPECT_THROW(parse({t(TokenType::KW_PRINT,"print"),
                        t(TokenType::STAR,"*"),
                        t(TokenType::NUMBER,"5",5.0), semi(), eof()}), ParseError);
}

TEST(ParserUnit, BlockStmt) {
    auto stmts = parse({t(TokenType::LEFT_BRACE,"{"),
                        t(TokenType::KW_PRINT,"print"),t(TokenType::NUMBER,"1",1.0),semi(),
                        t(TokenType::RIGHT_BRACE,"}"),eof()});
    auto* blk = dynamic_cast<BlockStmt*>(stmts[0].get());
    ASSERT_NE(blk, nullptr);
    EXPECT_EQ((int)blk->statements.size(), 1);
}
TEST(ParserUnit, ForStmt) {
    auto stmts = parse({
        t(TokenType::KW_FOR,"for"), t(TokenType::LEFT_PAREN,"("),
        t(TokenType::KW_VAR,"var"), t(TokenType::IDENTIFIER,"i"),
        t(TokenType::EQUAL,"="),    t(TokenType::NUMBER,"0",0.0), semi(),
        t(TokenType::IDENTIFIER,"i"),t(TokenType::LESS,"<"),
        t(TokenType::NUMBER,"3",3.0),semi(),
        t(TokenType::IDENTIFIER,"i"),t(TokenType::EQUAL,"="),
        t(TokenType::IDENTIFIER,"i"),t(TokenType::PLUS,"+"),
        t(TokenType::NUMBER,"1",1.0),t(TokenType::RIGHT_PAREN,")"),
        t(TokenType::KW_PRINT,"print"),t(TokenType::IDENTIFIER,"i"),semi(),eof()
    });
    EXPECT_NE(dynamic_cast<ForStmt*>(stmts[0].get()), nullptr);
}

TEST(ParserUnit, DanglingElse) {
    // if(true) if(false) print 1; else print 2;
    // else → 안쪽 if에 결합
    auto stmts = parse({
        t(TokenType::KW_IF,"if"),      t(TokenType::LEFT_PAREN,"("),
        t(TokenType::KW_TRUE,"true"),  t(TokenType::RIGHT_PAREN,")"),
        t(TokenType::KW_IF,"if"),      t(TokenType::LEFT_PAREN,"("),
        t(TokenType::KW_FALSE,"false"),t(TokenType::RIGHT_PAREN,")"),
        t(TokenType::KW_PRINT,"print"),t(TokenType::NUMBER,"1",1.0),semi(),
        t(TokenType::KW_ELSE,"else"),
        t(TokenType::KW_PRINT,"print"),t(TokenType::NUMBER,"2",2.0),semi(),eof()
    });
    auto* outer = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(outer, nullptr);
    EXPECT_EQ(outer->elseBranch, nullptr);  // 바깥 if → else 없음
    auto* inner = dynamic_cast<IfStmt*>(outer->thenBranch.get());
    ASSERT_NE(inner, nullptr);
    EXPECT_NE(inner->elseBranch, nullptr);  // 안쪽 if → else 있음
}

TEST(ParserUnit, PrintStmt) {
    auto stmts = parse({t(TokenType::KW_PRINT,"print"),
                        t(TokenType::NUMBER,"5",5.0), semi(), eof()});
    EXPECT_NE(dynamic_cast<PrintStmt*>(stmts[0].get()), nullptr);
}
TEST(ParserUnit, VarDecl_WithInit) {
    auto stmts = parse({t(TokenType::KW_VAR,"var"),
                        t(TokenType::IDENTIFIER,"a"),
                        t(TokenType::EQUAL,"="),
                        t(TokenType::NUMBER,"10",10.0), semi(), eof()});
    auto* vs = dynamic_cast<VarStmt*>(stmts[0].get());
    ASSERT_NE(vs, nullptr);
    EXPECT_EQ(vs->name.lexeme, "a");
    EXPECT_NE(vs->initializer, nullptr);
}
TEST(ParserUnit, VarDecl_NoInit) {
    auto stmts = parse({t(TokenType::KW_VAR,"var"),
                        t(TokenType::IDENTIFIER,"x"), semi(), eof()});
    auto* vs = dynamic_cast<VarStmt*>(stmts[0].get());
    ASSERT_NE(vs, nullptr);
    EXPECT_EQ(vs->initializer, nullptr);
}

TEST(ParserUnit, VariableRef) {
    auto stmts = parse({t(TokenType::IDENTIFIER,"a"), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<VariableExpr*>(es->expression.get()), nullptr);
}
TEST(ParserUnit, Assignment) {
    auto stmts = parse({t(TokenType::IDENTIFIER,"a"), t(TokenType::EQUAL,"="),
                        t(TokenType::NUMBER,"5",5.0), semi(), eof()});
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* asg = dynamic_cast<AssignExpr*>(es->expression.get());
    ASSERT_NE(asg, nullptr);
    EXPECT_EQ(asg->name.lexeme, "a");
}
TEST(ParserUnit, InvalidAssignTarget_Throws) {
    EXPECT_THROW(parse({t(TokenType::NUMBER,"1",1.0), t(TokenType::PLUS,"+"),
                        t(TokenType::NUMBER,"2",2.0), t(TokenType::EQUAL,"="),
                        t(TokenType::NUMBER,"3",3.0), semi(), eof()}), ParseError);
}

// ── Ch.2 함수 선언 테스트 ─────────────────────────────────────
TEST(ParserTest, FunctionDecl_NoParams) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func greet() { print \"hi\"; }");
    auto stmts  = parser.parse(std::move(tokens));
    ASSERT_EQ(stmts.size(), 1u);
    EXPECT_NE(dynamic_cast<FunctionStmt*>(stmts[0].get()), nullptr);
}
TEST(ParserTest, FunctionDecl_WithParams) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func add(a, b) { return a; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->params.size(), 2u);
}
TEST(ParserTest, FunctionDecl_MultipleParams) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f(a, b, c) { return a; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->params.size(), 3u);
    EXPECT_EQ(fn->params[0].lexeme, "a");
    EXPECT_EQ(fn->params[1].lexeme, "b");
    EXPECT_EQ(fn->params[2].lexeme, "c");
}
TEST(ParserTest, FunctionDecl_WithBody) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { var x = 1; return x; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->name.lexeme, "f");
    EXPECT_EQ(fn->body.size(), 2u);
}
TEST(ParserTest, FunctionDecl_Name) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func myFunc() { }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->name.lexeme, "myFunc");
    EXPECT_EQ(fn->params.size(), 0u);
    EXPECT_EQ(fn->body.size(), 0u);
}

// ── Ch.2 함수 호출 테스트 ─────────────────────────────────────
TEST(ParserTest, CallExpr_NoArgs) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("greet();");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<CallExpr*>(es->expression.get()), nullptr);
}
TEST(ParserTest, CallExpr_WithArgs) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("add(1, 2);");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es   = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* call = dynamic_cast<CallExpr*>(es->expression.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->args.size(), 2u);
}
TEST(ParserTest, CallExpr_MultipleArgs) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("f(1, 2, 3);");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es   = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* call = dynamic_cast<CallExpr*>(es->expression.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->args.size(), 3u);
}
TEST(ParserTest, CallExpr_NestedCall) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("f(g());");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es    = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* outer = dynamic_cast<CallExpr*>(es->expression.get());
    ASSERT_NE(outer, nullptr);
    ASSERT_EQ(outer->args.size(), 1u);
    EXPECT_NE(dynamic_cast<CallExpr*>(outer->args[0].get()), nullptr);
}

// ── Ch.2 return 문 테스트 ─────────────────────────────────────
TEST(ParserTest, ReturnStmt_WithValue) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { return 5; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn  = dynamic_cast<FunctionStmt*>(stmts[0].get());
    auto* ret = dynamic_cast<ReturnStmt*>(fn->body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_NE(ret->value, nullptr);
}
TEST(ParserTest, ReturnStmt_Void) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { return; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn  = dynamic_cast<FunctionStmt*>(stmts[0].get());
    auto* ret = dynamic_cast<ReturnStmt*>(fn->body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret->value, nullptr);
}
TEST(ParserTest, ReturnStmt_Keyword) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { return 42; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn  = dynamic_cast<FunctionStmt*>(stmts[0].get());
    auto* ret = dynamic_cast<ReturnStmt*>(fn->body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret->keyword.type, TokenType::KW_RETURN);
    EXPECT_EQ(ret->keyword.lexeme, "return");
}

TEST(ParserUnit, Addition) {
    auto stmts = parse({t(TokenType::NUMBER,"1",1.0),
                        t(TokenType::PLUS,"+"),
                        t(TokenType::NUMBER,"2",2.0), semi(), eof()});
    auto* bin = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->expression.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op.type, TokenType::PLUS);
}
TEST(ParserUnit, Precedence_MulBeforeAdd) {
    // 1 + 2 * 3 → right 쪽이 Binary(*)
    auto stmts = parse({t(TokenType::NUMBER,"1",1.0), t(TokenType::PLUS,"+"),
                        t(TokenType::NUMBER,"2",2.0), t(TokenType::STAR,"*"),
                        t(TokenType::NUMBER,"3",3.0), semi(), eof()});
    auto* add = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->expression.get());
    ASSERT_NE(add, nullptr);
    EXPECT_EQ(add->op.type, TokenType::PLUS);
    auto* mul = dynamic_cast<BinaryExpr*>(add->right.get());
    ASSERT_NE(mul, nullptr);
    EXPECT_EQ(mul->op.type, TokenType::STAR);
}
TEST(ParserUnit, LeftAssociativity) {
    // 10 - 4 - 3 → left 쪽이 Binary(-)
    auto stmts = parse({t(TokenType::NUMBER,"10",10.0), t(TokenType::MINUS,"-"),
                        t(TokenType::NUMBER,"4",4.0),   t(TokenType::MINUS,"-"),
                        t(TokenType::NUMBER,"3",3.0),   semi(), eof()});
    auto* outer = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->expression.get());
    ASSERT_NE(outer, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(outer->left.get()), nullptr);
}
TEST(ParserUnit, Comparison_Less) {
    auto stmts = parse({t(TokenType::NUMBER,"1",1.0), t(TokenType::LESS,"<"),
                        t(TokenType::NUMBER,"2",2.0), semi(), eof()});
    auto* bin = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->expression.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op.type, TokenType::LESS);
}

// ── Real Lexer 통합 테스트 ────────────────────────────────
// Lexer(실제) + Parser(실제) / Checker·Interpreter는 Mock으로 격리
TEST(RealLexerParser, NumberLiteral_PassesThrough) {
    auto mc = std::make_unique<MockChecker>();
    EXPECT_CALL(*mc, check(_)).Times(1);
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
                        std::move(mc), std::move(mi));
    EXPECT_NO_THROW(factory.run("5;"));
}

TEST(RealLexerParser, PrintStmt_NoThrow) {
    auto mc = std::make_unique<MockChecker>();
    EXPECT_CALL(*mc, check(_)).Times(1);
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
                        std::move(mc), std::move(mi));
    EXPECT_NO_THROW(factory.run("print 42;"));
}

TEST(RealLexerParser, VarDecl_NoThrow) {
    auto mc = std::make_unique<MockChecker>();
    EXPECT_CALL(*mc, check(_)).Times(1);
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(1);

    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
                        std::move(mc), std::move(mi));
    EXPECT_NO_THROW(factory.run("var x = 10;"));
}

TEST(RealLexerParser, ParseError_MissingSemicolon_Throws) {
    auto mc = std::make_unique<MockChecker>();
    EXPECT_CALL(*mc, check(_)).Times(0);
    auto mi = std::make_unique<MockInterpreter>();
    EXPECT_CALL(*mi, interpret(_)).Times(0);

    LangFactory factory(std::make_unique<Lexer>(), std::make_unique<Parser>(),
                        std::move(mc), std::move(mi));
    EXPECT_THROW(factory.run("print 5"), ParseError);
}
