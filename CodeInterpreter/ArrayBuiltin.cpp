#include "ArrayBuiltin.h"
#include "RuntimeError.h"
#include "Value.h"

// TODO (D): Array(n) → [nil * n] 생성 구현
Value ArrayBuiltin::call(Interpreter&, const std::vector<Value>& args) {
    // 1. args[0]이 double인지 확인 (아니면 RuntimeError)
    // 2. n < 0 이면 RuntimeError
    // 3. shared_ptr<vector<Value>>(n, nil) 반환
    throw RuntimeError("미구현: ArrayBuiltin::call()");
}
