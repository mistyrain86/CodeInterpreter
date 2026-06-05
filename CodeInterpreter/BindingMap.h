#pragma once
#include <unordered_map>

struct Expr;

// Ch.4 정적 바인딩: Expr 포인터 → 스코프 거리
// Resolver가 생성 → Interpreter::setBindings()로 전달
using BindingMap = std::unordered_map<const Expr*, int>;
