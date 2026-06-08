#include "Shell.h"
#include "Debugger.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

namespace {
void printPrompt(bool multiLine) {
    std::cout << (multiLine ? "... " : "> ");
    std::cout.flush();
}

void runWithErrors(LangFactory& factory, const std::string& source,
                   const std::function<void(int)>& onError) {
    try {
        factory.run(source);
    }
    catch (const ParseError& e)         { std::cerr << "[구문 오류] "   << e.what() << "\n"; onError(1); }
    catch (const CheckError& e)         { std::cerr << "[의미 오류] "   << e.what() << "\n"; onError(2); }
    catch (const RuntimeError& e)       { std::cerr << "[런타임 오류] " << e.what() << "\n"; onError(3); }
    catch (const std::runtime_error& e) { std::cerr << "[오류] "        << e.what() << "\n"; onError(4); }
}
}

void Shell::runRepl() {
    std::cout << "CodeFab Interpreter (REPL 모드)\n";
    std::cout << "종료: exit 또는 quit\n";

    LangFactory factory;
    std::string  line;
    std::ostringstream oss;

    auto flushBuffer = [&] {
        runSource(factory, oss.str());
        oss.str(""); oss.clear();
    };

    while (true) {
        printPrompt(!oss.str().empty());

        if (!std::getline(std::cin, line)) {
            if (!oss.str().empty()) runSource(factory, oss.str());
            break;
        }
        if (line == "exit" || line == "quit") break;

        if (line.empty()) {
            if (!oss.str().empty()) flushBuffer();
        }
        else {
            oss << line << '\n';
        }
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
    runWithErrors(factory, ss.str(), [](int code) { std::exit(code); });
}

void Shell::runDebug(const std::string& path) {
    Debugger debugger(path);
    debugger.run();
}

void Shell::runSource(LangFactory& factory, const std::string& source) {
    runWithErrors(factory, source, [](int) {});
}
