#pragma once
#include "Value.h"

// 함수 return문 실행 시 throw되어 콜 스택을 자동 언와인딩
// ICallable::call() 에서 catch 후 반환값 추출
struct ReturnSignal {
    Value value;
    explicit ReturnSignal(Value v) : value(std::move(v)) {}
};
