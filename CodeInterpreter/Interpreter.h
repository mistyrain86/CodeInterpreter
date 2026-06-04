#pragma once
#include <memory>
#include <string>
#include <vector>
#include "IInterpreter.h"
#include "Environment.h"
#include "Expr.h"
#include "ExprVisitor.h"
#include "RuntimeError.h"
#include "Stmt.h"
#include "StmtVisitor.h"
#include "Value.h"

class Interpreter : public IInterpreter
                  , public ExprVisitor
                  , public StmtVisitor {
public:
    Interpreter();
    void interpret(const std::vector<StmtPtr>& stmts) override;

    // ExprVisitor
    Value visitLiteral (LiteralExpr&)  override;
    Value visitGrouping(GroupingExpr&) override;
    Value visitUnary   (UnaryExpr&)    override;
    Value visitBinary  (BinaryExpr&)   override;
    Value visitVariable(VariableExpr&) override;
    Value visitAssign  (AssignExpr&)   override;

    // StmtVisitor
    void visitExprStmt (ExprStmt&)  override;
    void visitPrintStmt(PrintStmt&) override;
    void visitVarStmt  (VarStmt&)   override;
    void visitBlockStmt(BlockStmt&) override;
    void visitIfStmt   (IfStmt&)    override;
    void visitForStmt  (ForStmt&)   override;

    // 외부 공개 (LangFunction 등에서 활용)
    Value       evaluate(Expr& expr);
    void        execute(Stmt& stmt);
    void        executeBlock(const std::vector<StmtPtr>& stmts,
                             std::shared_ptr<Environment> env);
    bool        isTruthy(const Value& val) const;
    std::string stringify(const Value& val) const;
    std::shared_ptr<Environment> currentEnv() const { return m_currentEnv; }

private:
    std::shared_ptr<Environment> m_currentEnv;
    void        checkNumericOperand(const Value& val, int line) const;
    void        checkNumericPair(const Value& l, const Value& r, int line) const;
};
