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

    while (true) {
        std::cout << "> ";
        std::cout.flush();

        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        runSource(factory, line);
    }
}

void Shell::runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[오류] 파일을 찾을 수 없습니다: " << path << "\n";
        std::exit(1);
    }
    std::cout << "CodeFab Interpreter (FILE 모드)\n";
    std::cout << "[FILE] 소스코드 로딩: " << path << "\n";
    std::ostringstream ss;
    ss << file.rdbuf();

    LangFactory factory;
    try {
        factory.run(ss.str());
    }
    catch (const ParseError& e)        { std::cerr << "[구문 오류] "   << e.what() << "\n"; std::exit(1); }
    catch (const CheckError& e)        { std::cerr << "[의미 오류] "   << e.what() << "\n"; std::exit(2); }
    catch (const RuntimeError& e)      { std::cerr << "[런타임 오류] " << e.what() << "\n"; std::exit(3); }
    catch (const std::runtime_error& e){ std::cerr << "[오류] "        << e.what() << "\n"; std::exit(4); }
}

void Shell::runDebug(const std::string& path) {
    Debugger debugger(path);
    debugger.run();
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
