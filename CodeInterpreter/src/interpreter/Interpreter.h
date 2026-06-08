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
    Value visitCallExpr    (CallExpr&)      override;
    Value visitIndexGetExpr(IndexGetExpr&)  override;
    Value visitIndexSetExpr(IndexSetExpr&)  override;

    // StmtVisitor
    void visitExprStmt    (ExprStmt&)     override;
    void visitPrintStmt   (PrintStmt&)    override;
    void visitVarStmt     (VarStmt&)      override;
    void visitBlockStmt   (BlockStmt&)    override;
    void visitIfStmt      (IfStmt&)       override;
    void visitForStmt     (ForStmt&)      override;
    void visitFunctionStmt(FunctionStmt&) override;
    void visitReturnStmt  (ReturnStmt&)   override;

    // 외부 공개 (LangFunction 등에서 활용)
    Value       evaluate(Expr& expr);
    void        execute(Stmt& stmt);
    void        executeBlock(const std::vector<StmtPtr>& stmts,
                             std::shared_ptr<Environment> env);
    bool        isTruthy(const Value& val) const;
    std::string stringify(const Value& val) const;
    std::shared_ptr<Environment> currentEnv() const { return m_currentEnv; }

    void setBindings(const BindingMap* b) override { m_bindings = b; }
    std::vector<std::string> globalNames() const override;

    using BindingMap = ::BindingMap;
    using StmtHook = std::function<void(Stmt&)>;
    void setStmtHook(StmtHook hook) { m_stmtHook = std::move(hook); }

    // 디버거용: 현재 execute() 호출 깊이 (1=최상위, 2=블록 내부, ...)
    int executeDepth() const { return m_executeDepth; }

    // 정적 바인딩 검증용 Spy — 테스트에서 setSpy() 주입 후 경로 추적
    struct BindingSpy {
        int m_bindingHits = 0;  // scope stack O(1) 직접 접근 경로 (정적 바인딩)
        int m_chainWalks  = 0;  // get 경로 (환경 체인 탐색)
        void reset() { m_bindingHits = m_chainWalks = 0; }
    };
    void setSpy(BindingSpy* spy) { m_spy = spy; }

    // 상수 합치기 최적화 검증용 Spy — visitBinary 호출 횟수 추적
    struct OpSpy {
        int m_binaryOpCount = 0;
        void reset() { m_binaryOpCount = 0; }
    };
    void setOpSpy(OpSpy* spy) { m_opSpy = spy; }

    // 클로저 호출 시 scope stack 재구성용 — LangFunction::call()에서 사용
    std::vector<Environment*> saveScopeStack() const { return m_scopeStack; }
    void restoreScopeStack(std::vector<Environment*> saved) { m_scopeStack = std::move(saved); }
    void rebuildScopeStackFromClosure(Environment* closure);

private:
    std::shared_ptr<Environment>  m_currentEnv;
    std::vector<Environment*>     m_scopeStack;   // 평탄화 스코프 스택 — O(1) 직접 접근용
    const BindingMap*             m_bindings     = nullptr;
    StmtHook                      m_stmtHook;
    int                           m_executeDepth = 0;
    BindingSpy*                   m_spy          = nullptr;
    OpSpy*                        m_opSpy        = nullptr;

    using BinaryOpFn = std::function<Value(const Value&, const Value&, int)>;
    std::unordered_map<int, BinaryOpFn> m_binaryOps;
    void initBinaryOps();

    void        checkNumericOperand(const Value& val, int line) const;
    void        checkNumericPair(const Value& l, const Value& r, int line) const;
    std::optional<int> lookupBinding(const Expr* expr) const;
};
