#pragma once

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Expr.h"

struct Closure;
struct Distribution;

using Value = std::variant<
    double, 
    std::string, 
    std::shared_ptr<Closure>, 
    std::shared_ptr<Distribution>
>;

using Env = std::unordered_map<std::string, Value>;
using Address = std::vector<std::string>;

struct Closure {
    std::vector<Expr> params;
    std::vector<Expr> body;
    Env env;

    std::string getParamName(int index) const {
        return std::get<SymbolNode>(params[index].value).name;
    }
};

inline bool isPrimitive(const Value& val) {
    return std::holds_alternative<std::string>(val);
}

inline std::string getPrimitiveName(const Value& val) {
    return std::get<std::string>(val);
}

inline bool isClosure(const Value& val) {
    return std::holds_alternative<std::shared_ptr<Closure>>(val);
}

inline std::shared_ptr<Closure> getClosure(const Value& val) {
    return std::get<std::shared_ptr<Closure>>(val);
}

inline bool isTruthy(const Value& val) {
    if (std::holds_alternative<double>(val)) {
        return std::get<double>(val) != 0.0; 
    }

    return true; 
}