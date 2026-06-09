#pragma once
#include <istream>
#include <string>
#include "LangFactory.h"

class Shell {
public:
    void runRepl();
    void runFile(const std::string& path);
    void runDebug(const std::string& path);
    void runFromSource(const std::string& rawSource, const std::string& label);
    void runFileStream(std::istream& in, const std::string& label);

private:
    void runSource(LangFactory& factory, const std::string& source);
};
