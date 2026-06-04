#include <iostream>
#include <sstream>
#include <string>
#include "LangFactory.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define IS_INTERACTIVE() (_isatty(_fileno(stdin)))
#else
#include <unistd.h>
#define IS_INTERACTIVE() (isatty(fileno(stdin)))
#endif

#if !_DEBUG

static void runSource(const std::string& source) {
    LangFactory factory;
    try {
        factory.run(source);
    } catch (const ParseError& e) {
        std::cerr << "[구문 오류] " << e.what() << '\n';
    } catch (const CheckError& e) {
        std::cerr << "[의미 오류] " << e.what() << '\n';
    } catch (const RuntimeError& e) {
        std::cerr << "[런타임 오류] " << e.what() << '\n';
    } catch (const std::runtime_error& e) {
        std::cerr << "[오류] " << e.what() << '\n';
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (IS_INTERACTIVE()) {
        std::cout << "CodeFab Interpreter\n";
        std::cout << "여러 줄 입력 후 빈 줄을 입력하면 실행됩니다. 종료: Ctrl+Z (Windows) / Ctrl+D (Linux)\n";

        std::string line;
        std::ostringstream oss;

        while (true) {
            std::cout << (oss.str().empty() ? ">>> " : "... ");
            std::cout.flush();

            if (!std::getline(std::cin, line)) {
                if (!oss.str().empty()) runSource(oss.str());
                break;
            }

            if (line.empty()) {
                if (!oss.str().empty()) {
                    runSource(oss.str());
                    oss.str("");
                    oss.clear();
                }
            } else {
                oss << line << '\n';
            }
        }
    } else {
        // 파이프 / 파일 입력 모드
        std::ostringstream oss;
        std::string line;
        while (std::getline(std::cin, line))
            oss << line << '\n';

        LangFactory factory;
        try {
            factory.run(oss.str());
        } catch (const ParseError& e) {
            std::cerr << "[구문 오류] " << e.what() << '\n';
            return 1;
        } catch (const CheckError& e) {
            std::cerr << "[의미 오류] " << e.what() << '\n';
            return 2;
        } catch (const RuntimeError& e) {
            std::cerr << "[런타임 오류] " << e.what() << '\n';
            return 3;
        } catch (const std::runtime_error& e) {
            std::cerr << "[오류] " << e.what() << '\n';
            return 4;
        }
    }
    return 0;
}
#endif
