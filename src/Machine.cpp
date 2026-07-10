#include "Machine.h"
#include "Primitives.h"
#include <stdexcept>
#include <string>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Message Machine::resume() {
    while (!C.empty()) {
        Instruction instr = C.back();
        C.pop_back();

        std::optional<Message> msg = std::visit(overloaded {
            [this](EvInstr& i) { return stepEvaluate(i); },
            [this](CallkInstr& i) { return stepCallContinuation(i); },
            [this](LetkInstr& i) { return stepLetContinuation(i); }, 
            [this](DiscardInstr& i) -> std::optional<Message> { popValue(); return std::nullopt; },
            [](auto& i) -> std::optional<Message> { 
                throw std::runtime_error("Instruction handler not implemented."); 
            }
        }, instr);

        if (msg.has_value()) {
            return msg.value();
        }
    }

    Value final_result = V.empty() ? Value(0.0) : V.back();
    return DoneMsg{final_result, this};
}

// STEP METHODS

std::optional<Message> Machine::stepEvaluate(EvInstr& instr) {
    std::visit(overloaded {
        [this](double d) { evalLiteral(d); },
        [this, &instr](SymbolNode& sym) { evalSymbol(sym, instr.env); },
        [this, &instr](std::vector<Expr>& list) { evalList(list, instr.env, instr.addr); },
        [this, &instr](std::shared_ptr<LetNode>& let_node) { evalLetNode(let_node, instr.env, instr.addr); },
        [&instr](auto& other) {
            throw std::runtime_error("Expression type not implemented. Variant index: " + std::to_string(instr.e.value.index()));
        }
    }, instr.e.value);

    return std::nullopt; 
}

std::optional<Message> Machine::stepCallContinuation(CallkInstr& instr) {
    std::vector<Value> args = popArguments(instr.n_args);
    Value func = popValue();

    applyFunction(func, args);

    return std::nullopt;
}

std::optional<Message> Machine::stepLetContinuation(LetkInstr& instr) {
    Value val = popValue();
    std::string var_name = getBindingName(instr.binds, instr.i);

    Env new_env = instr.env;
    new_env[var_name] = val;

    int next_i = instr.i + 1;

    if (hasBinding(instr.binds, next_i)) {
        C.push_back(LetkInstr{instr.binds, next_i, instr.body, new_env, instr.addr});
        C.push_back(EvInstr{getBindingValue(instr.binds, next_i), new_env, extendAddress(instr.addr, "let_" + std::to_string(next_i * 2))});
    } else {
        pushBody(instr.body, new_env, instr.addr);
    }

    return std::nullopt;
}

// EVAL METHODS

void Machine::evalLiteral(double d) {
    V.push_back(d);
}

void Machine::evalSymbol(const SymbolNode& sym, const Env& current_env) {
    if (current_env.count(sym.name)) {
        V.push_back(current_env.at(sym.name));
    } 
    else if (PRIMITIVES.count(sym.name)) {
        V.push_back(sym.name); 
    } 
    else {
        throw std::runtime_error("NameError: " + sym.name);
    }
}

void Machine::evalList(const std::vector<Expr>& list, const Env& env, const Address& addr) {
    if (list.empty()) return;

    C.push_back(CallkInstr{ static_cast<int>(list.size()) - 1, addr });

    for (int i = list.size() - 1; i > 0; --i) {
        C.push_back(EvInstr{list[i], env, extendAddress(addr, std::to_string(i - 1))});
    }

    C.push_back(EvInstr{list[0], env, extendAddress(addr, "fn")});
}


void Machine::evalLetNode(const std::shared_ptr<LetNode>& let_node, const Env& env, const Address& addr) {
    if (hasBinding(let_node->binds, 0)) {
        C.push_back(LetkInstr{let_node->binds, 0, let_node->body, env, addr});
        C.push_back(EvInstr{getBindingValue(let_node->binds, 0), env, extendAddress(addr, "let_0")});
    } else {
        pushBody(let_node->body, env, addr);
    }
}

// AUX - DECLARATIVE METHODS

std::vector<Value> Machine::popArguments(int n) {
    std::vector<Value> args(n);
    for (int i = n - 1; i >= 0; --i) {
        args[i] = popValue();
    }
    return args;
}

Value Machine::popValue() {
    Value val = V.back();
    V.pop_back();
    return val;
}

Address Machine::extendAddress(Address addr, const std::string& suffix) {
    addr.push_back(suffix);
    return addr; 
}

void Machine::applyFunction(const Value& func, const std::vector<Value>& args) {
    if (isPrimitive(func)) {
        std::string prim_name = getPrimitiveName(func);
        if (PRIMITIVES.count(prim_name)) {
            V.push_back(PRIMITIVES.at(prim_name)(args));
        } else {
            throw std::runtime_error("Not a function: " + prim_name);
        }
    } else {
        throw std::runtime_error("Function application for non-primitives not implemented yet.");
    }
}

bool Machine::hasBinding(const std::vector<Expr>& binds, int pair_index) {
    return (2 * pair_index) < binds.size();
}

std::string Machine::getBindingName(const std::vector<Expr>& binds, int pair_index) {
    return std::get<SymbolNode>(binds[2 * pair_index].value).name;
}

Expr Machine::getBindingValue(const std::vector<Expr>& binds, int pair_index) {
    return binds[(2 * pair_index) + 1];
}

void Machine::pushBody(const std::vector<Expr>& body, const Env& env, const Address& addr) {
    C.push_back(EvInstr{body.back(), env, extendAddress(addr, "body_" + std::to_string(body.size() - 1))});
    
    for (int i = static_cast<int>(body.size()) - 2; i >= 0; --i) {
        C.push_back(DiscardInstr{});
        C.push_back(EvInstr{body[i], env, extendAddress(addr, "body_" + std::to_string(i))});
    }
}

bool Machine::isPrimitive(const Value& func) {
    return std::holds_alternative<std::string>(func);
}

std::string Machine::getPrimitiveName(const Value& func) {
    return std::get<std::string>(func);
}