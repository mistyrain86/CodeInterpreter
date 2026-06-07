#pragma once
#include <string>
#include "LangFactory.h"

class Shell {
public:
    void runRepl();
    void runFile(const std::string& path);
    void runDebug(const std::string& path);

private:
    void runSource(LangFactory& factory, const std::string& source);
};
