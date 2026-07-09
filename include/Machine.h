#pragma once
#include <vector>
#include <optional>
#include "Value.h"
#include "Message.h"
#include "Instruction.h"

struct Machine {
    std::vector<Instruction> C;
    std::vector<Value> V;
    Env env;
    double log_w = 0.0;

    Message resume();
    Machine fork() const;

    std::optional<Message> operator()(const EvInstr& instr);
    std::optional<Message> operator()(const IfkInstr& instr);
    std::optional<Message> operator()(const LetkInstr& instr);
    std::optional<Message> operator()(const CallkInstr& instr);
    std::optional<Message> operator()(const SamplekInstr& instr);
    std::optional<Message> operator()(const ObservekInstr& instr);
    std::optional<Message> operator()(const DiscardInstr& instr);

    void operator()(double d);
    void operator()(const SymbolNode& s);
    void operator()(const auto& node);
};