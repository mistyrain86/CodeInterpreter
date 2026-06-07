#pragma once
#include <stdexcept>
#include <string>

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg) : std::runtime_error(msg) {}
};
