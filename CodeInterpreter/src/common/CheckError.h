#pragma once
#include <stdexcept>
#include <string>

class CheckError : public std::runtime_error {
public:
    explicit CheckError(const std::string& msg) : std::runtime_error(msg) {}
};
