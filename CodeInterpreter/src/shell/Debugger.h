#pragma once
#include <climits>
#include <set>
#include <string>
#include <vector>
#include "LangFactory.h"

class Interpreter;

struct DebugSessionExit {};  // exit/quit 커맨드 시 throw — 스택 정상 해제 후 종료

class Debugger {
public:
    explicit Debugger(const std::string& path);

    void run();

private:
    std::string              m_path;
    std::vector<std::string> m_sourceLines;
    std::set<int>            m_breakpoints;
    std::set<std::string>    m_watches;
    bool                     m_stepMode  = true;
    int                      m_nextDepth = INT_MAX;  // step: INT_MAX, next: 현재 depth

    void onBeforeStmt(Stmt& stmt, Interpreter& interp);
    void processCommand(const std::string& input, Interpreter& interp);

    void cmdBreak     (int line);
    void cmdRemove    (int line);
    void cmdBreakpoints();
    void cmdWatch     (const std::string& var);
    void cmdUnwatch   (const std::string& var);
    void cmdWatched   (Interpreter& interp);
    void cmdInspect   (Interpreter& interp);
    void printWatches (Interpreter& interp);
};
