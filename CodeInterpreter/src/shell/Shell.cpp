#include "Shell.h"
#include "Debugger.h"
#include "ShellUtils.h"
#include <functional>
#include <iostream>

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
    catch (const ParseError& e)         { printError(e); onError(ParseErrorCode); }
    catch (const CheckError& e)         { printError(e); onError(CheckErrorCode); }
    catch (const RuntimeError& e)       { printError(e); onError(RuntimeErrorCode); }
    catch (const std::runtime_error& e) { printError(e); onError(InternalErrorCode); }
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
    runFileStream(file, path);
}

void Shell::runFileStream(std::istream& in, const std::string& label) {
    std::ostringstream ss;
    ss << in.rdbuf();
    runFromSource(ss.str(), label);
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

    forEachChunk(lines, [&](int startLine, const std::string& src) -> bool {
        std::string source(startLine, '\n');
        source += src;
        runWithErrors(factory, source, [&](int) { hasError = true; });
        return !hasError;
    });
}

void Shell::runDebug(const std::string& path) {
    Debugger debugger(path);
    debugger.run();
}

void Shell::runSource(LangFactory& factory, const std::string& source) {
    runWithErrors(factory, source, [](int) {});
}
