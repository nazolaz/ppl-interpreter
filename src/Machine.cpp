#include "../include/Machine.h"

Message Machine::resume() {
    while (!C.empty()) {
        Instruction instr = C.back();
        C.pop_back();

        std::optional<Message> msg = std::visit(*this, instr);
        if (msg.has_value()) {
            return msg.value();
        }
    }
    return DoneMsg{V.back(), this};
}

std::optional<Message> Machine::operator()(const EvInstr& instr) {
    std::visit(*this, instr.e.value);
    return std::nullopt;
}

std::optional<Message> Machine::operator()(const IfkInstr& instr) { return std::nullopt; }
std::optional<Message> Machine::operator()(const LetkInstr& instr) { return std::nullopt; }
std::optional<Message> Machine::operator()(const CallkInstr& instr) { return std::nullopt; }
std::optional<Message> Machine::operator()(const SamplekInstr& instr) { return std::nullopt; }
std::optional<Message> Machine::operator()(const ObservekInstr& instr) { return std::nullopt; }
std::optional<Message> Machine::operator()(const DiscardInstr& instr) { return std::nullopt; }

void Machine::operator()(double d) {
    V.push_back(d);
}

void Machine::operator()(const SymbolNode& s) {
    V.push_back(env.at(s.name));
}

void Machine::operator()(const auto& node) {}

Machine Machine::fork() const {
    return *this;
}