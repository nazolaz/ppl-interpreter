#include "Primitives.h"
#include "Distribution.h" 
#include <cmath>
#include <stdexcept>
#include <memory>
#include <vector>

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
    }},

    // --- CONSTRUCTORES DE DISTRIBUCIONES ---
    {"normal", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Normal>(get_num(args[0]), get_num(args[1]));
    }},
    {"bernoulli", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Bernoulli>(get_num(args[0]));
    }},
    {"flip", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Bernoulli>(get_num(args[0]));
    }},
    {"exponential", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Exponential>(get_num(args[0]));
    }},
    {"uniform", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Uniform>(get_num(args[0]), get_num(args[1]));
    }},
    {"uniform-continuous", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Uniform>(get_num(args[0]), get_num(args[1]));
    }},
    {"poisson", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<Poisson>(get_num(args[0]));
    }},
    {"beta", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<BetaDist>(get_num(args[0]), get_num(args[1]));
    }},
    {"gamma", [](const std::vector<Value>& args) -> Value {
        return std::make_shared<GammaDist>(get_num(args[0]), get_num(args[1]));
    }},
    
    {"discrete", [](const std::vector<Value>& args) -> Value {
        std::vector<double> probs;
        probs.reserve(args.size());
        for (const auto& arg : args) {
            probs.push_back(get_num(arg));
        }
        return std::make_shared<Discrete>(probs);
    }},
    {"categorical", [](const std::vector<Value>& args) -> Value {
        std::vector<double> probs;
        probs.reserve(args.size());
        for (const auto& arg : args) {
            probs.push_back(get_num(arg));
        }
        return std::make_shared<Discrete>(probs);
    }}
};

bool is_primitive(const std::string& name) {
    return PRIMITIVES.find(name) != PRIMITIVES.end();
}