#include "ArrayBuiltin.h"
#include "RuntimeError.h"
#include "Value.h"

Value ArrayBuiltin::call(Interpreter&, const std::vector<Value>& args) {
    if (!std::holds_alternative<double>(args[0]))
        throw RuntimeError("런타임 오류: 배열 크기는 숫자여야 합니다.");
    int n = static_cast<int>(std::get<double>(args[0]));
    if (n < 0)
        throw RuntimeError("런타임 오류: 배열 크기는 0 이상이어야 합니다.");
    if (n > 1000000)
        throw RuntimeError("런타임 오류: 배열 크기가 너무 큽니다.");
    return Value{std::make_shared<std::vector<Value>>(
        n, Value{std::monostate{}})};
}
