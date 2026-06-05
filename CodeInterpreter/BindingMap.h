#pragma once
#include <unordered_map>

struct Expr;

using BindingMap = std::unordered_map<const Expr*, int>;
