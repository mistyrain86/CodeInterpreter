#pragma once
#include <climits>
#include <set>
#include <string>
#include <vector>
#include "LangFactory.h"

class Interpreter;

struct DebugSessionExit {};

class Debugger {
public:
    explicit Debugger(const std::string& path);

    // 디버그 루프 진입 (소스 로드 → 한 Stmt씩 실행)
    void run();

private:
    std::string              m_path;
    std::vector<std::string> m_sourceLines;             // 소스 줄 캐시 (줄 번호 표시용)
    std::set<int>            m_breakpoints;             // 설정된 줄 번호
    std::set<std::string>    m_watches;                 // 감시 중인 변수명
    bool                     m_stepMode     = true;     // false = continue 중
    int                      m_nextDepth    = INT_MAX;  // step: INT_MAX, next: 현재 depth
    int                      m_lastStmtLine = 0;        // 직전 stmt 줄 번호 (범위 bp 판정용)

    void onBeforeStmt(Stmt& stmt, Interpreter& interp);

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
