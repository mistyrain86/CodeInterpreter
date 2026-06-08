#define NOMINMAX
#include <iostream>
#include <string>
#include "Shell.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if !_DEBUG
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Shell shell;

    if (argc == 1) {
        shell.runRepl();
        return 0;
    }

    if (argc == 3) {
        std::string mode = argv[1];
        std::string path = argv[2];

        if (mode == "run") {
            shell.runFile(path);
            return 0;
        }
        if (mode == "debug") {
            shell.runDebug(path);
            return 0;
        }
    }

    std::cerr << "사용법:\n";
    std::cerr << "  (인자 없음)          -- REPL 모드\n";
    std::cerr << "  run <파일 경로>   -- 파일 실행 모드\n";
    std::cerr << "  debug <파일 경로> -- 디버그 모드\n";
    return 1;
}
#endif
