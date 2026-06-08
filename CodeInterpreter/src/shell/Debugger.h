#pragma once
#include <climits>
#include <set>
#include <string>
#include <vector>
#include "LangFactory.h"

class Interpreter;

struct DebugSessionExit {};  // exit/quit 커맨드 시 throw — 스택 정상 해제 후 종료

// Ch.5 디버거 — Stmt 단위 stepping, watch, inspect, breakpoint
class Debugger {
public:
    explicit Debugger(const std::string& path);

    // 디버그 루프 진입 (소스 로드 → 한 Stmt씩 실행)
    void run();

private:
    std::string              m_path;
    std::vector<std::string> m_sourceLines;         // 소스 줄 캐시 (줄 번호 표시용)
    std::set<int>            m_breakpoints;          // 설정된 줄 번호
    std::set<std::string>    m_watches;              // 감시 중인 변수명
    bool                     m_stepMode  = true;     // false = continue 중
    int                      m_nextDepth = INT_MAX;  // step: INT_MAX, next: 현재 depth

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
    void cmdWatched   (Interpreter& interp);   // watched: 감시 변수 목록 + 현재 값
    void cmdInspect   (Interpreter& interp);   // inspect: 스코프 전체 변수 + 값 + 타입
    void printWatches (Interpreter& interp);   // 정지 시 자동 출력
};
