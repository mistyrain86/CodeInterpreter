#pragma once
#include <set>
#include <string>
#include <vector>
#include "LangFactory.h"

class Interpreter;

// Ch.5 디버거 — Stmt 단위 stepping, watch, inspect, breakpoint
// A가 Debugger.cpp 에서 구현
class Debugger {
public:
    explicit Debugger(const std::string& path);

    // 디버그 루프 진입 (소스 로드 → 한 Stmt씩 실행)
    void run();

private:
    std::string              m_path;
    std::set<int>            m_breakpoints;  // 설정된 줄 번호
    std::set<std::string>    m_watches;      // 감시 중인 변수명
    bool                     m_stepMode = true;  // false = continue 중

    // Interpreter StmtHook — 각 Stmt 실행 전 호출
    void onBeforeStmt(Stmt& stmt, Interpreter& interp);

    // 커맨드 처리
    void processCommand(const std::string& input, Interpreter& interp);

    // 개별 커맨드
    void cmdBreak     (int line);
    void cmdRemove    (int line);
    void cmdBreakpoints();
    void cmdWatch     (const std::string& var);
    void cmdUnwatch   (const std::string& var);
    void cmdWatches   ();
    void cmdInspect   (Interpreter& interp);
    void printWatches (Interpreter& interp);
};
