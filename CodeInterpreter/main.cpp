#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

#if !_DEBUG
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    std::cout << "CodeFab - Phase 0 빌드 확인용\n";
    return 0;
}
#endif
