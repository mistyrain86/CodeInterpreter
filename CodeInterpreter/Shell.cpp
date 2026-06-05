#include "Shell.h"
#include "LangFactory.h"
#include "Debugger.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#include <fstream>
#include <iostream>
#include <sstream>

// TODO (A): 아래 3개 메서드를 구현하세요.

void Shell::runRepl() {
    // TODO:
    // 1. "CodeFab Interpreter (REPL)" 출력
    // 2. 동일 LangFactory 인스턴스 재사용 (전역변수 유지)
    // 3. "> " 프롬프트 출력 후 한 줄 읽기
    // 4. "exit" / "quit" 입력 시 종료
    // 5. factory.run(line) 실행, 에러 출력 후 계속
}

void Shell::runFile(const std::string& path) {
    // TODO:
    // 1. 파일 열기, 없으면 "[오류] 파일을 찾을 수 없습니다: path" 출력 후 exit(1)
    // 2. 전체 내용 읽기
    // 3. factory.run(source) 실행
    // 4. 에러 발생 시 메시지 출력 후 exit(코드)
}

void Shell::runDebug(const std::string& path) {
    // TODO:
    // 1. Debugger debugger(path) 생성
    // 2. debugger.run() 실행
}
