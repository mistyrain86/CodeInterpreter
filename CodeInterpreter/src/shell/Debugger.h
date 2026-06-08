#pragma once
#include <climits>
#include <istream>
#include <set>
#include <string>
#include <vector>
#include "LangFactory.h"

class Interpreter;

struct DebugSessionExit {};

class Debugger {
public:
    explicit Debugger(const std::string& path);

    void run();
    void run(const std::string& source, std::istream& cmdIn);

private:
    std::string              m_path;
    std::vector<std::string> m_sourceLines;
    std::set<int>            m_breakpoints;
    std::set<std::string>    m_watches;
    bool                     m_stepMode     = true;
    int                      m_nextDepth    = INT_MAX;  // step: INT_MAX, next: 현재 depth
    int                      m_lastStmtLine = 0;

    void onBeforeStmt(Stmt& stmt, Interpreter& interp, std::istream& cmdIn);

    void processCommand(const std::string& input, Interpreter& interp);

    void cmdBreak     (int line);
    void cmdRemove    (int line);
    void cmdBreakpoints();
    void cmdWatch     (const std::string& var);
    void cmdUnwatch   (const std::string& var);
    void cmdWatched   (Interpreter& interp);   // 감시 변수 목록 + 현재 값
    void cmdInspect   (Interpreter& interp);   // 스코프 전체 변수 + 값 + 타입
    void printWatches (Interpreter& interp);   // 정지 시 자동 출력
};
