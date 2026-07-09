#pragma once
#include <vector>
#include <optional>
#include "Value.h"
#include "Message.h"
#include "Instruction.h"

class Machine {
public:
    std::vector<Instruction> C;
    std::vector<Value> V;
    Env env;
    double log_w = 0.0;

    Machine() = default;

    Message resume();
    Machine fork() const;

private:
    std::optional<Message> stepEvaluate(EvInstr& instr);
    
    void evalLiteral(double d);
    void evalSymbol(const SymbolNode& sym, const Env& current_env);
};