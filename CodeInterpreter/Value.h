#pragma once
#include <string>
#include <variant>

using Value = std::variant<std::monostate, double, std::string, bool>;
