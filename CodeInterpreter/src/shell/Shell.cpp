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

enum ExitCode { ParseErrorCode, CheckErrorCode, RuntimeErrorCode, InternalErrorCode };

void runWithErrors(LangFactory& factory, const std::string& source,
                   const std::function<void(int)>& onError) {
    try {
        factory.run(source);
    }
    catch (const ParseError& e)         { std::cerr << "[구문 오류] "   << e.what() << "\n"; onError(ParseErrorCode); }
    catch (const CheckError& e)         { std::cerr << "[의미 오류] "   << e.what() << "\n"; onError(CheckErrorCode); }
    catch (const RuntimeError& e)       { std::cerr << "[런타임 오류] " << e.what() << "\n"; onError(RuntimeErrorCode); }
    catch (const std::runtime_error& e) { std::cerr << "[오류] "        << e.what() << "\n"; onError(InternalErrorCode); }
}
}

void Shell::runRepl() {
    std::cout << "CodeFab Interpreter (REPL 모드)\n";
    std::cout << "종료: exit 또는 quit\n";

    LangFactory factory;
    std::string  line;

    while (true) {
        printPrompt(false);

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
    std::ostringstream ss;
    ss << file.rdbuf();
    runFromSource(ss.str(), path);
}

void Shell::runFromSource(const std::string& rawSource, const std::string& label) {
    std::cout << "CodeFab Interpreter (FILE 모드)\n";
    std::cout << "[FILE] 소스코드 로딩: " << label << "\n";

    std::vector<std::string> lines;
    std::istringstream stream(rawSource);
    std::string ln;
    while (std::getline(stream, ln)) lines.push_back(ln);

    LangFactory factory;
    bool hasError = false;
    std::ostringstream current;
    int chunkStart = 0;

    for (int i = 0; i <= (int)lines.size(); i++) {
        if (hasError) break;
        bool isBlank = (i == (int)lines.size()) ||
                       lines[i].find_first_not_of(" \t\r\n") == std::string::npos;
        if (isBlank) {
            if (!current.str().empty()) {
                std::string source(chunkStart, '\n');
                source += current.str();
                runWithErrors(factory, source, [&](int) { hasError = true; });
                current.str(""); current.clear();
            }
            chunkStart = i + 1;
        } else {
            current << lines[i] << '\n';
        }
    }
}

void Shell::runDebug(const std::string& path) {
    Debugger debugger(path);
    debugger.run();
}

void Shell::runSource(LangFactory& factory, const std::string& source) {
    runWithErrors(factory, source, [](int) {});
}
