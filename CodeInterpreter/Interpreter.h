#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
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
    Value visitLiteral     (LiteralExpr&)   override;
    Value visitGrouping    (GroupingExpr&)  override;
    Value visitUnary       (UnaryExpr&)     override;
    Value visitBinary      (BinaryExpr&)    override;
    Value visitVariable    (VariableExpr&)  override;
    Value visitAssign      (AssignExpr&)    override;
    Value visitCallExpr    (CallExpr&)      override;  // Ch.2 — D 구현
    Value visitIndexGetExpr(IndexGetExpr&)  override;  // Ch.3 — D 구현
    Value visitIndexSetExpr(IndexSetExpr&)  override;  // Ch.3 — D 구현

    // StmtVisitor
    void visitExprStmt    (ExprStmt&)     override;
    void visitPrintStmt   (PrintStmt&)    override;
    void visitVarStmt     (VarStmt&)      override;
    void visitBlockStmt   (BlockStmt&)    override;
    void visitIfStmt      (IfStmt&)       override;
    void visitForStmt     (ForStmt&)      override;
    void visitFunctionStmt(FunctionStmt&) override;  // Ch.2 — D 구현
    void visitReturnStmt  (ReturnStmt&)   override;  // Ch.2 — D 구현

    // 외부 공개 (LangFunction 등에서 활용)
    Value       evaluate(Expr& expr);
    void        execute(Stmt& stmt);
    void        executeBlock(const std::vector<StmtPtr>& stmts,
                             std::shared_ptr<Environment> env);
    bool        isTruthy(const Value& val) const;
    std::string stringify(const Value& val) const;
    std::shared_ptr<Environment> currentEnv() const { return m_currentEnv; }

    // Ch.4 정적 바인딩
    using BindingMap = std::unordered_map<const Expr*, int>;
    void setBindings(const BindingMap* b) { m_bindings = b; }

    // Ch.5 디버거 훅
    using StmtHook = std::function<void(Stmt&)>;
    void setStmtHook(StmtHook hook) { m_stmtHook = std::move(hook); }

private:
    std::shared_ptr<Environment> m_currentEnv;
    const BindingMap*            m_bindings  = nullptr;
    StmtHook                     m_stmtHook;
    void        checkNumericOperand(const Value& val, int line) const;
    void        checkNumericPair(const Value& l, const Value& r, int line) const;
    std::optional<int> lookupBinding(const Expr* expr) const;
};
