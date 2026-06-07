#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>

class ICallable;

// 배열 타입: shared_ptr로 참조 의미론 제공
using ArrayType = std::shared_ptr<std::vector<struct Value>>;

struct Value : std::variant<
    std::monostate,                // nil
    double,                        // 숫자
    std::string,                   // 문자열
    bool,                          // 불리언
    ArrayType,                     // 배열   (Phase: 정적 배열)
    std::shared_ptr<ICallable>     // 함수   (Phase: function)
> {
    using variant::variant;
    using variant::operator=;
};
