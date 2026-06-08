#define NOMINMAX
#include <iostream>
#include <string>
#include <limits>
#include "Shell.h"

#ifdef _WIN32
#include <windows.h>
#endif

#if !_DEBUG
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Shell shell;

    while (true) {
        std::cout << "=================================\n";
        std::cout << "       Factory 모드 선택         \n";
        std::cout << "=================================\n";
        std::cout << " 1. REPL 모드\n";
        std::cout << " 2. 파일 실행 모드\n";
        std::cout << " 3. 디버그 모드\n";
        std::cout << " 4. 종료\n";
        std::cout << "=================================\n";
        std::cout << "원하는 모드의 번호를 입력하세요: ";

        int choice;
        std::cin >> choice;

        if (std::cin.fail()) {
            std::cout << "\n[오류] 숫자가 아닌 잘못된 문자가 입력되었습니다. 다시 시도하세요.\n\n";

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            continue;
        }

        std::string filename;

        switch (choice) {
        case 1:
            std::cout << "\nREPL 모드를 시작합니다...\n";
            shell.runRepl();
            break;

        case 2:
            std::cout << "실행할 파일명을 입력하세요 (예: test.txt): ";
            std::cin >> filename;
            std::cout << filename << " 파일을 실행합니다...\n\n";
            shell.runFile(filename);
            break;

        case 3:
            std::cout << "\n아직 미구현된 기능입니다.\n\n";
            break;

            std::cout << "디버그할 파일명을 입력하세요 (예: test.txt): ";
            std::cin >> filename;
            std::cout << filename << " 파일의 디버깅을 시작합니다...\n\n";
            shell.runDebug(filename);
            break;

        case 4:
            std::cout << "\n프로그램을 종료합니다.\n";
            return 0;

        default:
            std::cout << "\n[오류] 1부터 4 사이의 숫자만 입력할 수 있습니다.\n\n";
            break;
        }
    }

    return 0;
}
#endif
