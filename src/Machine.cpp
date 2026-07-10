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
            [this](IfkInstr& i) { return stepIfContinuation(i); },
            [this](SamplekInstr& i) { return stepSampleContinuation(i); },
            [this](ObservekInstr& i) { return stepObserveContinuation(i); },
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
    return DoneMsg{final_result};
}

void Machine::load_ast(const Expr& ast, const Env& env) {
    C.push_back(EvInstr{ast, env, Address{}});
}

void Machine::send(const Value& val) {
    V.push_back(val);
}

void Machine::add_log_weight(double weight) {
    log_w += weight;
}

double Machine::get_log_weight() const {
    return log_w;
}

// STEP METHODS

std::optional<Message> Machine::stepEvaluate(EvInstr& instr) {
    std::visit(overloaded {
        [this](double d) { evalLiteral(d); },
        [this, &instr](SymbolNode& sym) { evalSymbol(sym, instr.env); },
        [this, &instr](std::vector<Expr>& list) { evalList(list, instr.env, instr.addr); },
        [this, &instr](std::shared_ptr<LetNode>& let_node) { evalLetNode(let_node, instr.env, instr.addr); },
        [this, &instr](std::shared_ptr<FnNode>& fn_node) { evalFnNode(fn_node, instr.env); },
        [this, &instr](std::shared_ptr<IfNode>& if_node) { evalIfNode(if_node, instr.env, instr.addr); },
        [&instr](auto& other) {
            throw std::runtime_error("Expression type not implemented. Variant index: " + std::to_string(instr.e.value.index()));
        }
    }, instr.e.value);

    return std::nullopt; 
}

std::optional<Message> Machine::stepCallContinuation(CallkInstr& instr) {
    std::vector<Value> args = popArguments(instr.n_args);
    Value func = popValue();

    applyFunction(func, args, instr.addr);

    return std::nullopt;
}

std::optional<Message> Machine::stepIfContinuation(IfkInstr& instr) {
    Value test_result = popValue();
    
    if (isTruthy(test_result)) {
        C.push_back(EvInstr{*instr.then_branch, instr.env, extendAddress(instr.addr, "if_then")});
    } else {
        C.push_back(EvInstr{*instr.else_branch, instr.env, extendAddress(instr.addr, "if_else")});
    }
    
    return std::nullopt;
}

std::optional<Message> Machine::stepLetContinuation(LetkInstr& instr) {
    Value val = popValue();
    std::string var_name = instr.getBindingName(instr.i);

    Env new_env = instr.env;
    new_env[var_name] = val;

    int next_i = instr.i + 1;

    if (instr.hasBinding(next_i)) {
        C.push_back(LetkInstr{instr.binds, next_i, instr.body, new_env, instr.addr});
        C.push_back(EvInstr{instr.getBindingValue(next_i), new_env, extendAddress(instr.addr, "let_" + std::to_string(next_i * 2))});
    } else {
        pushBody(instr.body, new_env, instr.addr);
    }

    return std::nullopt;
}

std::optional<Message> Machine::stepSampleContinuation(SamplekInstr& instr) {
    Value dist_val = popValue();
    auto dist = std::get<std::shared_ptr<Distribution>>(dist_val);
    
    return SampleMsg{instr.addr, dist};
}

std::optional<Message> Machine::stepObserveContinuation(ObservekInstr& instr) {
    Value y_val = popValue();
    Value dist_val = popValue();
    
    double y = std::get<double>(y_val);
    auto dist = std::get<std::shared_ptr<Distribution>>(dist_val);
    
    return ObserveMsg{instr.addr, dist, y};
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

void Machine::evalIfNode(const std::shared_ptr<IfNode>& if_node, const Env& env, const Address& addr) {
    C.push_back(IfkInstr{if_node->then_branch, if_node->else_branch, env, addr});
    C.push_back(EvInstr{*if_node->test, env, extendAddress(addr, "if_test")});
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
    if (let_node->hasBinding(0)) {
        C.push_back(LetkInstr{let_node->binds, 0, let_node->body, env, addr});
        C.push_back(EvInstr{let_node->getBindingValue(0), env, extendAddress(addr, "let_0")});
    } else {
        pushBody(let_node->body, env, addr);
    }
}

void Machine::evalFnNode(const std::shared_ptr<FnNode>& fn_node, const Env& env) {
    auto closure = std::make_shared<Closure>();
    
    closure->params = fn_node->params;
    closure->body = fn_node->body;
    
    closure->env = env;
    
    V.push_back(closure);
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

void Machine::applyFunction(const Value& func, const std::vector<Value>& args, const Address& addr) {
    if (isPrimitive(func)) {
        std::string prim_name = getPrimitiveName(func);
        
        if (prim_name == "sample") {
            V.push_back(args[0]); 
            C.push_back(SamplekInstr{addr});
            return;
        } 
        else if (prim_name == "observe") {
            V.push_back(args[0]); 
            V.push_back(args[1]); 
            C.push_back(ObservekInstr{addr});
            return;
        }

        applyPrimitive(func, args); 
    } else if (isClosure(func)) {
        applyClosure(func, args, addr);
    } else {
        throw std::runtime_error("Function application for this type is not implemented.");
    }
}

void Machine::pushBody(const std::vector<Expr>& body, const Env& env, const Address& addr) {
    C.push_back(EvInstr{body.back(), env, extendAddress(addr, "body_" + std::to_string(body.size() - 1))});
    
    for (int i = static_cast<int>(body.size()) - 2; i >= 0; --i) {
        C.push_back(DiscardInstr{});
        C.push_back(EvInstr{body[i], env, extendAddress(addr, "body_" + std::to_string(i))});
    }
}

void Machine::applyPrimitive(const Value& func, const std::vector<Value>& args) {
    std::string prim_name = getPrimitiveName(func);
    if (PRIMITIVES.count(prim_name)) {
        V.push_back(PRIMITIVES.at(prim_name)(args));
    } else {
        throw std::runtime_error("Not a function: " + prim_name);
    }
}

void Machine::applyClosure(const Value& func, const std::vector<Value>& args, const Address& addr) {
    auto closure = getClosure(func);
    
    if (args.size() != closure->params.size()) {
        throw std::runtime_error("Arity mismatch: expected " + std::to_string(closure->params.size()) + " arguments.");
    }

    Env execution_env = closure->env;

    for (size_t i = 0; i < args.size(); ++i) {
        execution_env[closure->getParamName(i)] = args[i];
    }

    pushBody(closure->body, execution_env, extendAddress(addr, "apply"));
}

