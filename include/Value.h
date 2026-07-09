#pragma once

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Expr.h"

struct Closure;
struct Distribution;

using Value = std::variant<double, std::string, std::shared_ptr<Closure>, std::shared_ptr<Distribution>>;
using Env = std::unordered_map<std::string, Value>;
using Address = std::vector<std::string>;

struct Closure {
    std::vector<std::string> params;
    std::vector<Expr> body;
    Env env;
};