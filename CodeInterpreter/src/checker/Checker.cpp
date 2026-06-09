#include <cassert>
#include "Checker.h"

namespace {
static std::string checkErr(int line, const std::string& msg) {
    return "[라인 " + std::to_string(line) + "] 의미 오류: " + msg;
}
}

void Checker::check(const std::vector<StmtPtr>& stmts) {
    beginScope();
    for (const auto& name : m_knownGlobals)
        m_scopes.back()[name] = true;
    beginScope();

    Token arrayBuiltin{TokenType::IDENTIFIER, "Array", std::monostate{}, 0};
    declare(arrayBuiltin);
    define(arrayBuiltin);

    m_inUserCode = true;
    checkStmts(stmts);
    m_inUserCode = false;

    endScope();
    endScope();
}

void Checker::registerGlobal(const std::string& name) {
    m_knownGlobals.insert(name);
}

void Checker::checkStmts(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) s->accept(*this);
}

void Checker::visitVarStmt(VarStmt& s) {
    declare(s.m_name);
    if (s.m_initializer) s.m_initializer->acceptVoid(*this);
    define(s.m_name);
}

void Checker::visitBlockStmt(BlockStmt& s) {
    beginScope();
    checkStmts(s.m_statements);
    endScope();
}

void Checker::visitIfStmt(IfStmt& s) {
    s.m_condition->acceptVoid(*this);
    s.m_thenBranch->accept(*this);
    if (s.m_elseBranch) s.m_elseBranch->accept(*this);
}

void Checker::visitForStmt(ForStmt& s) {
    beginScope();
    if (s.m_initializer) s.m_initializer->accept(*this);
    if (s.m_condition)   s.m_condition->acceptVoid(*this);
    if (s.m_increment)   s.m_increment->acceptVoid(*this);
    if (s.m_body)        s.m_body->accept(*this);
    endScope();
}

void Checker::visitPrintStmt(PrintStmt& s) {
    s.m_expression->acceptVoid(*this);
}

void Checker::visitExprStmt(ExprStmt& s) {
    s.m_expression->acceptVoid(*this);
}

void Checker::visitFunctionStmt(FunctionStmt& s) {
    declare(s.m_name);
    define(s.m_name);

    std::unordered_set<std::string> seen;
    for (const auto& param : s.m_params) {
        if (seen.count(param.lexeme))
            throw CheckError(checkErr(param.line, "파라미터 이름이 중복됩니다. ('" + param.lexeme + "')"));
        seen.insert(param.lexeme);
    }

    m_functionDepth++;
    beginScope();
    for (const auto& param : s.m_params) {
        declare(param);
        define(param);
    }
    checkStmts(s.m_body);
    endScope();
    m_functionDepth--;
}

void Checker::visitReturnStmt(ReturnStmt& s) {
    if (m_functionDepth == 0)
        throw CheckError(checkErr(s.m_keyword.line, "함수 외부에서 return을 사용할 수 없습니다."));
    if (s.m_value) s.m_value->acceptVoid(*this);
}

void Checker::visitGrouping(GroupingExpr& e) {
    e.expression->acceptVoid(*this);
}

void Checker::visitUnary(UnaryExpr& e) {
    e.right->acceptVoid(*this);
}

void Checker::visitBinary(BinaryExpr& e) {
    e.left->acceptVoid(*this); e.right->acceptVoid(*this);
}

void Checker::visitLogical(LogicalExpr& e) {
    e.left->acceptVoid(*this); e.right->acceptVoid(*this);
}

void Checker::visitVariable(VariableExpr& e) {
    resolveVar(e.name.lexeme, e.name.line);
}

void Checker::visitAssign(AssignExpr& e) {
    e.value->acceptVoid(*this);
    resolveVar(e.name.lexeme, e.name.line);
}

void Checker::visitCallExpr(CallExpr& e) {
    e.callee->acceptVoid(*this);
    for (auto& arg : e.args) arg->acceptVoid(*this);
}

void Checker::visitIndexGetExpr(IndexGetExpr& e) {
    e.object->acceptVoid(*this);
    e.index->acceptVoid(*this);
}

void Checker::visitIndexSetExpr(IndexSetExpr& e) {
    e.object->acceptVoid(*this);
    e.index->acceptVoid(*this);
    e.value->acceptVoid(*this);
}

void Checker::beginScope() { m_scopes.emplace_back(); }
void Checker::endScope()   { m_scopes.pop_back(); }

void Checker::declare(const Token& name) {
    auto& scope      = m_scopes.back();
    bool  inScope    = scope.count(name.lexeme) > 0;
    // 전역 레벨(beginScope 2회)에서 이미 선언된 전역 변수 재선언
    bool  globalRedecl = m_inUserCode && m_scopes.size() == 2
                      && m_knownGlobals.count(name.lexeme) > 0;
    if (inScope || globalRedecl)
        throw CheckError(checkErr(name.line, "이미 이 스코프에 같은 이름의 변수가 있습니다. ('" + name.lexeme + "')"));
    scope[name.lexeme] = false;
}

void Checker::define(const Token& name) {
    m_scopes.back()[name.lexeme] = true;
}

void Checker::resolveVar(const std::string& name, int line) {
    for (int i = (int)m_scopes.size() - 1; i >= 0; i--) {
        auto it = m_scopes[i].find(name);
        if (it != m_scopes[i].end()) {
            if (!it->second)
                throw CheckError(checkErr(line, "자신의 초기화식에서 지역변수를 읽을 수 없습니다. ('" + name + "')"));
            return;
        }
    }
    throw CheckError(checkErr(line, "선언되지 않은 변수입니다. ('" + name + "')"));
}
