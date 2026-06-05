#include "Shell.h"
#include "Debugger.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#include <fstream>
#include <iostream>
#include <sstream>

void Shell::runRepl() {
    std::cout << "CodeFab Interpreter (REPL 모드)\n";
    std::cout << "종료: exit 또는 quit\n";

    LangFactory factory;
    std::string  line;
    std::ostringstream oss;

    while (true) {
        std::cout << (oss.str().empty() ? "> " : "... ");
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            if (!oss.str().empty()) runSource(factory, oss.str());
            break;
        }
        if (line == "exit" || line == "quit") break;

        if (line.empty()) {
            if (!oss.str().empty()) {
                runSource(factory, oss.str());
                oss.str(""); oss.clear();
            }
        }
        else {
            oss << line << '\n';
        }
    }
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

void Shell::runSource(LangFactory& factory, const std::string& source) {
    try {
        factory.run(source);
    }
    catch (const ParseError& e) { std::cerr << "[구문 오류] " << e.what() << "\n"; }
    catch (const CheckError& e) { std::cerr << "[의미 오류] " << e.what() << "\n"; }
    catch (const RuntimeError& e) { std::cerr << "[런타임 오류] " << e.what() << "\n"; }
    catch (const std::runtime_error& e) { std::cerr << "[오류] " << e.what() << "\n"; }
}
