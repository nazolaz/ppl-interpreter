#include "Primitives.h"
#include <cmath>
#include <stdexcept>

double get_num(const Value& v) {
    if (std::holds_alternative<double>(v)) {
        return std::get<double>(v);
    }
    throw std::runtime_error("Expected a number");
}

std::unordered_map<std::string, PrimitiveFunc> PRIMITIVES = {
    {"+", [](const std::vector<Value>& args) -> Value {
        double sum = 0;
        for (const auto& a : args) {
            sum += get_num(a);
        }
        return sum;
    }},
    {"-", [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return 0.0;
        if (args.size() == 1) return -get_num(args[0]);
        double res = get_num(args[0]);
        for (size_t i = 1; i < args.size(); ++i) {
            res -= get_num(args[i]);
        }
        return res;
    }},
    {"*", [](const std::vector<Value>& args) -> Value {
        double prod = 1.0;
        if (!args.empty()) {
            prod = get_num(args[0]);
            for (size_t i = 1; i < args.size(); ++i) {
                prod *= get_num(args[i]);
            }
        }
        return prod;
    }},
    {"/", [](const std::vector<Value>& args) -> Value {
        if (args.empty()) throw std::runtime_error("Division requires arguments");
        if (args.size() == 1) return 1.0 / get_num(args[0]);
        double res = get_num(args[0]);
        for (size_t i = 1; i < args.size(); ++i) {
            res /= get_num(args[i]);
        }
        return res;
    }},
    {"<", [](const std::vector<Value>& args) -> Value {
        return (get_num(args[0]) < get_num(args[1])) ? 1.0 : 0.0;
    }},
    {">", [](const std::vector<Value>& args) -> Value {
        return (get_num(args[0]) > get_num(args[1])) ? 1.0 : 0.0;
    }},
    {"=", [](const std::vector<Value>& args) -> Value {
        return (get_num(args[0]) == get_num(args[1])) ? 1.0 : 0.0;
    }}
};

bool is_primitive(const std::string& name) {
    return PRIMITIVES.find(name) != PRIMITIVES.end();
}