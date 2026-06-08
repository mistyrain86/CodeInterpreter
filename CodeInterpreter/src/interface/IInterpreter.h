#pragma once
#include <functional>
#include <string>
#include <vector>
#include "BindingMap.h"
#include "Stmt.h"

class IInterpreter {
public:
    virtual ~IInterpreter() = default;
    virtual void interpret(const std::vector<StmtPtr>& stmts) = 0;
    virtual void setBindings(const BindingMap* b) {}
    virtual std::vector<std::string> globalNames() const { return {}; }

    using StmtHook = std::function<void(Stmt&)>;
    virtual void setStmtHook(StmtHook hook) {}
    virtual int  executeDepth() const { return 0; }
};
