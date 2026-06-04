#include <iostream>
#include <sstream>
#include <string>
#include "LangFactory.h"
#include "ParseError.h"
#include "CheckError.h"
#include "RuntimeError.h"
#ifdef _WIN32
#include <windows.h>
#endif

#if !_DEBUG
int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
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
    return 0;
}
#endif
