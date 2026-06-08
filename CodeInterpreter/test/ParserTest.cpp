#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mocks.h"
#include "Lexer.h"
#include "Parser.h"
#include "LangFactory.h"
#include "TestUtils.h"
#include "TokenStreamBuilder.h"

using ::testing::_;
using ::testing::Return;

static std::vector<StmtPtr> parse(std::vector<Token> tokens) {
    return Parser().parse(std::move(tokens));
}

// ── 단위 테스트 ───────────────────────────────────────────
TEST(ParserUnit, StringLiteral) {
    auto stmts = parse(TokenStreamBuilder().string("hi").semicolon().eof().build());
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->m_expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<std::string>(lit->value), "hi");
}
TEST(ParserUnit, BoolTrue) {
    auto stmts = parse(TokenStreamBuilder().boolTrue().semicolon().eof().build());
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->m_expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<bool>(lit->value), true);
}
TEST(ParserUnit, BoolFalse) {
    auto stmts = parse(TokenStreamBuilder().boolFalse().semicolon().eof().build());
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->m_expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_EQ(std::get<bool>(lit->value), false);
}
TEST(ParserUnit, Grouping) {
    auto stmts = parse(TokenStreamBuilder().lparen().number(5.0).rparen().semicolon().eof().build());
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<GroupingExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserUnit, UnaryMinus) {
    auto stmts = parse(TokenStreamBuilder().minus().number(3.0).semicolon().eof().build());
    auto* un = dynamic_cast<UnaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->m_expression.get());
    ASSERT_NE(un, nullptr);
    EXPECT_EQ(un->op.type, TokenType::MINUS);
}
TEST(ParserUnit, UnaryBang) {
    auto stmts = parse(TokenStreamBuilder().bang().boolTrue().semicolon().eof().build());
    auto* un = dynamic_cast<UnaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->m_expression.get());
    ASSERT_NE(un, nullptr);
    EXPECT_EQ(un->op.type, TokenType::BANG);
}

TEST(ParserUnit, MissingSemicolon_Throws) {
    EXPECT_THROW(parse(TokenStreamBuilder().kwPrint().number(5.0).eof().build()), ParseError);
}
TEST(ParserUnit, MissingCloseParen_Throws) {
    EXPECT_THROW(parse(TokenStreamBuilder().kwPrint().lparen().number(1.0).semicolon().eof().build()), ParseError);
}
TEST(ParserUnit, ExpectExpression_Throws) {
    EXPECT_THROW(parse(TokenStreamBuilder().kwPrint().star().number(5.0).semicolon().eof().build()), ParseError);
}

TEST(ParserUnit, BlockStmt) {
    auto stmts = parse(TokenStreamBuilder()
        .lbrace().kwPrint().number(1.0).semicolon().rbrace().eof().build());
    auto* blk = dynamic_cast<BlockStmt*>(stmts[0].get());
    ASSERT_NE(blk, nullptr);
    EXPECT_EQ((int)blk->m_statements.size(), 1);
}
TEST(ParserUnit, ForStmt) {
    auto stmts = parse(TokenStreamBuilder()
        .kwFor().lparen()
            .kwVar().identifier("i").equal().number(0.0).semicolon()
            .identifier("i").less().number(3.0).semicolon()
            .identifier("i").equal().identifier("i").plus().number(1.0)
        .rparen()
        .kwPrint().identifier("i").semicolon()
        .eof().build());
    EXPECT_NE(dynamic_cast<ForStmt*>(stmts[0].get()), nullptr);
}

TEST(ParserUnit, DanglingElse) {
    // if(true) if(false) print 1; else print 2;
    // else → 안쪽 if에 결합
    auto stmts = parse(TokenStreamBuilder()
        .kwIf().lparen().boolTrue().rparen()
        .kwIf().lparen().boolFalse().rparen()
        .kwPrint().number(1.0).semicolon()
        .kwElse()
        .kwPrint().number(2.0).semicolon()
        .eof().build());
    auto* outer = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(outer, nullptr);
    EXPECT_EQ(outer->m_elseBranch, nullptr);  // 바깥 if → else 없음
    auto* inner = dynamic_cast<IfStmt*>(outer->m_thenBranch.get());
    ASSERT_NE(inner, nullptr);
    EXPECT_NE(inner->m_elseBranch, nullptr);  // 안쪽 if → else 있음
}

TEST(ParserUnit, PrintStmt) {
    auto stmts = parse(TokenStreamBuilder().kwPrint().number(5.0).semicolon().eof().build());
    EXPECT_NE(dynamic_cast<PrintStmt*>(stmts[0].get()), nullptr);
}
TEST(ParserUnit, VarDecl_WithInit) {
    auto stmts = parse(TokenStreamBuilder()
        .kwVar().identifier("a").equal().number(10.0).semicolon().eof().build());
    auto* vs = dynamic_cast<VarStmt*>(stmts[0].get());
    ASSERT_NE(vs, nullptr);
    EXPECT_EQ(vs->m_name.lexeme, "a");
    EXPECT_NE(vs->m_initializer, nullptr);
}
TEST(ParserUnit, VarDecl_NoInit) {
    auto stmts = parse(TokenStreamBuilder().kwVar().identifier("x").semicolon().eof().build());
    auto* vs = dynamic_cast<VarStmt*>(stmts[0].get());
    ASSERT_NE(vs, nullptr);
    EXPECT_EQ(vs->m_initializer, nullptr);
}

TEST(ParserUnit, VariableRef) {
    auto stmts = parse(TokenStreamBuilder().identifier("a").semicolon().eof().build());
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<VariableExpr*>(es->m_expression.get()), nullptr);
}
TEST(ParserUnit, Assignment) {
    auto stmts = parse(TokenStreamBuilder()
        .identifier("a").equal().number(5.0).semicolon().eof().build());
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* asg = dynamic_cast<AssignExpr*>(es->m_expression.get());
    ASSERT_NE(asg, nullptr);
    EXPECT_EQ(asg->name.lexeme, "a");
}
TEST(ParserUnit, InvalidAssignTarget_Throws) {
    EXPECT_THROW(parse(TokenStreamBuilder()
        .number(1.0).plus().number(2.0).equal().number(3.0).semicolon().eof().build()), ParseError);
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
    EXPECT_EQ(fn->m_params.size(), 2u);
}
TEST(ParserTest, FunctionDecl_MultipleParams) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f(a, b, c) { return a; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->m_params.size(), 3u);
    EXPECT_EQ(fn->m_params[0].lexeme, "a");
    EXPECT_EQ(fn->m_params[1].lexeme, "b");
    EXPECT_EQ(fn->m_params[2].lexeme, "c");
}
TEST(ParserTest, FunctionDecl_WithBody) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { var x = 1; return x; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->m_name.lexeme, "f");
    EXPECT_EQ(fn->m_body.size(), 2u);
}
TEST(ParserTest, FunctionDecl_Name) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func myFunc() { }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
    ASSERT_NE(fn, nullptr);
    EXPECT_EQ(fn->m_name.lexeme, "myFunc");
    EXPECT_EQ(fn->m_params.size(), 0u);
    EXPECT_EQ(fn->m_body.size(), 0u);
}

// ── Ch.2 함수 호출 테스트 ─────────────────────────────────────
TEST(ParserTest, CallExpr_NoArgs) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("greet();");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<CallExpr*>(es->m_expression.get()), nullptr);
}
TEST(ParserTest, CallExpr_WithArgs) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("add(1, 2);");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es   = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* call = dynamic_cast<CallExpr*>(es->m_expression.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->args.size(), 2u);
}
TEST(ParserTest, CallExpr_MultipleArgs) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("f(1, 2, 3);");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es   = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* call = dynamic_cast<CallExpr*>(es->m_expression.get());
    ASSERT_NE(call, nullptr);
    EXPECT_EQ(call->args.size(), 3u);
}
TEST(ParserTest, CallExpr_NestedCall) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("f(g());");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es    = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* outer = dynamic_cast<CallExpr*>(es->m_expression.get());
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
    auto* ret = dynamic_cast<ReturnStmt*>(fn->m_body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_NE(ret->m_value, nullptr);
}
TEST(ParserTest, ReturnStmt_Void) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { return; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn  = dynamic_cast<FunctionStmt*>(stmts[0].get());
    auto* ret = dynamic_cast<ReturnStmt*>(fn->m_body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret->m_value, nullptr);
}
TEST(ParserTest, ReturnStmt_Keyword) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func f() { return 42; }");
    auto stmts  = parser.parse(std::move(tokens));
    auto* fn  = dynamic_cast<FunctionStmt*>(stmts[0].get());
    auto* ret = dynamic_cast<ReturnStmt*>(fn->m_body[0].get());
    ASSERT_NE(ret, nullptr);
    EXPECT_EQ(ret->m_keyword.type, TokenType::KW_RETURN);
    EXPECT_EQ(ret->m_keyword.lexeme, "return");
}

// ── Ch.3 배열 인덱스 테스트 ───────────────────────────────────
TEST(ParserTest, IndexGetExpr) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("arr[0];");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<IndexGetExpr*>(es->m_expression.get()), nullptr);
}
TEST(ParserTest, IndexGetExpr_WithVar) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("arr[i];");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* idx = dynamic_cast<IndexGetExpr*>(es->m_expression.get());
    ASSERT_NE(idx, nullptr);
    EXPECT_NE(dynamic_cast<VariableExpr*>(idx->index.get()), nullptr);
}
TEST(ParserTest, IndexSetExpr) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("arr[0] = 5;");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<IndexSetExpr*>(es->m_expression.get()), nullptr);
}
TEST(ParserTest, IndexSetExpr_WithExpr) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("arr[0] = 1 + 2;");
    auto stmts  = parser.parse(std::move(tokens));
    auto* es  = dynamic_cast<ExprStmt*>(stmts[0].get());
    auto* set = dynamic_cast<IndexSetExpr*>(es->m_expression.get());
    ASSERT_NE(set, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(set->value.get()), nullptr);
}

// ── Ch.2/3 에러 케이스 테스트 ────────────────────────────────
TEST(ParserTest, FunctionDecl_MissingName_Throws) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("func () { }");
    EXPECT_THROW(parser.parse(std::move(tokens)), ParseError);
}
TEST(ParserTest, CallExpr_MissingCloseParen_Throws) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("f(1, 2;");
    EXPECT_THROW(parser.parse(std::move(tokens)), ParseError);
}
TEST(ParserTest, IndexExpr_MissingCloseBracket_Throws) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("arr[0;");
    EXPECT_THROW(parser.parse(std::move(tokens)), ParseError);
}
TEST(ParserTest, IndexExpr_EmptyIndex_Throws) {
    Lexer lexer; Parser parser;
    auto tokens = lexer.tokenize("arr[];");
    EXPECT_THROW(parser.parse(std::move(tokens)), ParseError);
}

TEST(ParserUnit, Precedence_MulBeforeAdd) {
    // 1 + 2 * 3 → right 쪽이 Binary(*)
    auto stmts = parse(TokenStreamBuilder()
        .number(1.0).plus().number(2.0).star().number(3.0).semicolon().eof().build());
    auto* add = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->m_expression.get());
    ASSERT_NE(add, nullptr);
    EXPECT_EQ(add->op.type, TokenType::PLUS);
    auto* mul = dynamic_cast<BinaryExpr*>(add->right.get());
    ASSERT_NE(mul, nullptr);
    EXPECT_EQ(mul->op.type, TokenType::STAR);
}
TEST(ParserUnit, LeftAssociativity) {
    // 10 - 4 - 3 → left 쪽이 Binary(-)
    auto stmts = parse(TokenStreamBuilder()
        .number(10.0).minus().number(4.0).minus().number(3.0).semicolon().eof().build());
    auto* outer = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->m_expression.get());
    ASSERT_NE(outer, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(outer->left.get()), nullptr);
}
TEST(ParserUnit, Comparison_Less) {
    auto stmts = parse(TokenStreamBuilder()
        .number(1.0).less().number(2.0).semicolon().eof().build());
    auto* bin = dynamic_cast<BinaryExpr*>(
        dynamic_cast<ExprStmt*>(stmts[0].get())->m_expression.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op.type, TokenType::LESS);
}

// ── Real Lexer 통합 테스트 ────────────────────────────────
// Lexer(실제) + Parser(실제) / Checker·Interpreter는 Mock으로 격리

class RealLexerParserFixture : public ::testing::Test {
protected:
    MockChecker*     m_mcRaw = nullptr;
    MockInterpreter* m_miRaw = nullptr;
    std::unique_ptr<LangFactory> m_factory;

    void SetUp() override {
        auto mc  = std::make_unique<MockChecker>();
        m_mcRaw  = mc.get();
        auto mi  = std::make_unique<MockInterpreter>();
        m_miRaw  = mi.get();
        m_factory = std::make_unique<LangFactory>(
            std::make_unique<Lexer>(), std::make_unique<Parser>(),
            std::move(mc), std::move(mi));
    }
};

TEST_F(RealLexerParserFixture, NumberLiteral_PassesThrough) {
    EXPECT_CALL(*m_mcRaw, check(_)).Times(1);
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(1);
    EXPECT_NO_THROW(m_factory->run("5;"));
}

TEST_F(RealLexerParserFixture, PrintStmt_NoThrow) {
    EXPECT_CALL(*m_mcRaw, check(_)).Times(1);
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(1);
    EXPECT_NO_THROW(m_factory->run("print 42;"));
}

TEST_F(RealLexerParserFixture, VarDecl_NoThrow) {
    EXPECT_CALL(*m_mcRaw, check(_)).Times(1);
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(1);
    EXPECT_NO_THROW(m_factory->run("var x = 10;"));
}

TEST_F(RealLexerParserFixture, ParseError_MissingSemicolon_Throws) {
    EXPECT_CALL(*m_mcRaw, check(_)).Times(0);
    EXPECT_CALL(*m_miRaw, interpret(_)).Times(0);
    EXPECT_THROW(m_factory->run("print 5"), ParseError);
}

// ── TokenStreamBuilder 활용 예시 ─────────────────────────
// 기존 t()/semi()/eof() 방식 대비 문법 흐름이 코드에 바로 드러난다.

TEST(ParserBuilder, NumberLiteral) {
    auto tokens = TokenStreamBuilder().number(5.0).semicolon().eof().build();
    auto stmts  = Parser().parse(std::move(tokens));
    auto* es    = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* lit = dynamic_cast<LiteralExpr*>(es->m_expression.get());
    ASSERT_NE(lit, nullptr);
    EXPECT_DOUBLE_EQ(std::get<double>(lit->value), 5.0);
}

TEST(ParserBuilder, Addition) {
    auto tokens = TokenStreamBuilder()
        .number(1.0).plus().number(2.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es   = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    auto* bin = dynamic_cast<BinaryExpr*>(es->m_expression.get());
    ASSERT_NE(bin, nullptr);
    EXPECT_EQ(bin->op.type, TokenType::PLUS);
}

TEST(ParserBuilder, VarDecl) {
    auto tokens = TokenStreamBuilder()
        .kwVar().identifier("x").equal().number(42.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* vs   = dynamic_cast<VarStmt*>(stmts[0].get());
    ASSERT_NE(vs, nullptr);
    EXPECT_EQ(vs->m_name.lexeme, "x");
    EXPECT_NE(vs->m_initializer, nullptr);
}

TEST(ParserBuilder, ForLoop) {
    // for (var i = 0; i < 3; i = i + 1) print i;
    auto tokens = TokenStreamBuilder()
        .kwFor().lparen()
            .kwVar().identifier("i").equal().number(0.0).semicolon()
            .identifier("i").less().number(3.0).semicolon()
            .identifier("i").equal()
                .identifier("i").plus().number(1.0)
        .rparen()
        .kwPrint().identifier("i").semicolon()
        .eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    EXPECT_NE(dynamic_cast<ForStmt*>(stmts[0].get()), nullptr);
}

TEST(ParserBuilder, IfStmt_ThenOnly) {
    auto tokens = TokenStreamBuilder()
        .kwIf().lparen().boolTrue().rparen()
        .kwPrint().number(1.0).semicolon()
        .eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    ASSERT_EQ(stmts.size(), 1u);
    auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_EQ(ifStmt->m_elseBranch, nullptr);
}

TEST(ParserBuilder, IfStmt_WithElse) {
    auto tokens = TokenStreamBuilder()
        .kwIf().lparen().boolFalse().rparen()
        .kwPrint().number(1.0).semicolon()
        .kwElse()
        .kwPrint().number(2.0).semicolon()
        .eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    ASSERT_EQ(stmts.size(), 1u);
    auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
    ASSERT_NE(ifStmt, nullptr);
    EXPECT_NE(ifStmt->m_elseBranch, nullptr);
}

TEST(ParserBuilder, EqualEqual_Expr) {
    auto tokens = TokenStreamBuilder()
        .number(1.0).equalEqual().number(1.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserBuilder, BangEqual_Expr) {
    auto tokens = TokenStreamBuilder()
        .number(1.0).bangEqual().number(2.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserBuilder, Greater_Comparison) {
    auto tokens = TokenStreamBuilder()
        .number(3.0).greater().number(1.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserBuilder, LessEqual_Comparison) {
    auto tokens = TokenStreamBuilder()
        .number(1.0).lessEqual().number(2.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserBuilder, GreaterEqual_Comparison) {
    auto tokens = TokenStreamBuilder()
        .number(2.0).greaterEqual().number(2.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserBuilder, Subtraction_Term) {
    auto tokens = TokenStreamBuilder()
        .number(5.0).minus().number(3.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}

TEST(ParserBuilder, Division_Factor) {
    auto tokens = TokenStreamBuilder()
        .number(6.0).slash().number(2.0).semicolon().eof().build();
    auto stmts = Parser().parse(std::move(tokens));
    auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
    ASSERT_NE(es, nullptr);
    EXPECT_NE(dynamic_cast<BinaryExpr*>(es->m_expression.get()), nullptr);
}
