#include "Machine.h"
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

std::optional<Message> Machine::stepEvaluate(EvInstr& instr) {
    std::visit(overloaded {
        [this](double d) { evalLiteral(d); },
        // FIX: Removed 'const' so C++ prioritizes this over auto&
        [this, &instr](SymbolNode& sym) { evalSymbol(sym, instr.env); },
        // Added the variant index to the error message to help debug future missing types!
        [&instr](auto& other) {
            throw std::runtime_error("Expression type not implemented. Variant index: " + std::to_string(instr.e.value.index()));
        }
    }, instr.e.value);

    return std::nullopt; 
}

void Machine::evalLiteral(double d) {
    V.push_back(d);
}

void Machine::evalSymbol(const SymbolNode& sym, const Env& current_env) {
    if (current_env.count(sym.name)) {
        V.push_back(current_env.at(sym.name));
    } else {
        throw std::runtime_error("NameError: " + sym.name);
    }
}