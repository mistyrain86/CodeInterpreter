#pragma once

struct ExprStmt;
struct PrintStmt;
struct VarStmt;
struct BlockStmt;
struct IfStmt;
struct ForStmt;
struct FunctionStmt;
struct ReturnStmt;

struct StmtVisitor {
    virtual ~StmtVisitor() = default;
    virtual void visitExprStmt    (ExprStmt&)     = 0;
    virtual void visitPrintStmt   (PrintStmt&)    = 0;
    virtual void visitVarStmt     (VarStmt&)      = 0;
    virtual void visitBlockStmt   (BlockStmt&)    = 0;
    virtual void visitIfStmt      (IfStmt&)       = 0;
    virtual void visitForStmt     (ForStmt&)      = 0;
    virtual void visitFunctionStmt(FunctionStmt&) = 0;
    virtual void visitReturnStmt  (ReturnStmt&)   = 0;
};
