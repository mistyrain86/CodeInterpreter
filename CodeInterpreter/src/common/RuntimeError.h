#pragma once
#include <stdexcept>
#include <string>

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
    static std::string format(int line, const std::string& msg) {
        return "[라인 " + std::to_string(line) + "] " + msg;
    }
};
