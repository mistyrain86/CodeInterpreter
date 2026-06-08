#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>

class ICallable;

// 배열 타입: shared_ptr로 참조 의미론 제공
using ArrayType = std::shared_ptr<std::vector<struct Value>>;

struct Value : std::variant<
    std::monostate,                // null
    double,
    std::string,
    bool,
    ArrayType,
    std::shared_ptr<ICallable>
> {
    using variant::variant;
    using variant::operator=;
};
