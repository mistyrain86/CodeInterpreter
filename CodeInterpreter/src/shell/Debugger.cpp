#include "Debugger.h"
#include "Interpreter.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#include <fstream>
#include <iostream>
#include <sstream>

// TODO (A): 아래 메서드들을 구현하세요.

Debugger::Debugger(const std::string& path) : m_path(path) {}

void Debugger::run() {
    // TODO:
    // 1. 파일 읽기 (없으면 오류)
    // 2. Lexer/Parser로 stmts 파싱
    // 3. Interpreter 생성, setStmtHook 설정
    // 4. "[DEBUG] 소스코드 로딩: path" 출력
    // 5. interpret(stmts) 실행
}

void Debugger::onBeforeStmt(Stmt& stmt, Interpreter& interp) {
    // TODO:
    // 1. 현재 줄 번호 확인 (stmt.getLine())
    // 2. breakpoint이거나 m_stepMode이면 정지
    // 3. "[DEBUG] N번째 줄에서 정지 → 소스" 출력
    // 4. printWatches 출력
    // 5. 커맨드 루프 (step/next → break, continue → m_stepMode=false + break)
}

void Debugger::processCommand(const std::string& input, Interpreter& interp) {
    // TODO: 각 커맨드 파싱 후 해당 메서드 호출
    // "break N", "remove N", "Breakpoints", "watch VAR", "unwatch VAR",
    // "watches", "inspect", "step"/"next", "continue"
}

void Debugger::cmdBreak(int line) {
    m_breakpoints.insert(line);
    std::cout << "[DEBUG] " << line << "번째 줄에 breakpoint 설정\n";
}
void Debugger::cmdRemove(int line) {
    m_breakpoints.erase(line);
    std::cout << "[DEBUG] " << line << "번째 줄 breakpoint 제거\n";
}
void Debugger::cmdBreakpoints() {
    std::cout << "[Breakpoints]\n";
    for (int bp : m_breakpoints) std::cout << "  줄 " << bp << "\n";
}
void Debugger::cmdWatch(const std::string& var) {
    m_watches.insert(var);
    std::cout << "[WATCH] " << var << " 감시 등록\n";
}
void Debugger::cmdUnwatch(const std::string& var) {
    m_watches.erase(var);
    std::cout << "[WATCH] " << var << " 감시 해제\n";
}
void Debugger::cmdWatches() {
    std::cout << "[감시 중인 변수]\n";
    for (auto& w : m_watches) std::cout << "  " << w << "\n";
}
void Debugger::cmdInspect(Interpreter& interp) {
    // TODO: interp.currentEnv()->printAll() 호출
    std::cout << "----------- 현재 스코프 변수 -----------\n";
}
void Debugger::printWatches(Interpreter& interp) {
    // TODO: m_watches의 각 변수를 interp.currentEnv()에서 조회하여 출력
}
