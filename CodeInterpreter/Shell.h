#pragma once
#include <string>

// Ch.5 공장 제어 쉘
// A가 Shell.cpp 에서 구현
class Shell {
public:
    // ./factory           → REPL 모드 (세션 전역변수 유지, exit/quit 종료)
    void runRepl();

    // ./factory run <파일> → 파일 모드 (파일 없음 오류, 에러 즉시 종료)
    void runFile(const std::string& path);

    // ./factory debug <파일> → 디버그 모드 (Debugger 위임)
    void runDebug(const std::string& path);
};
