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
    std::optional<Message> stepCallContinuation(CallkInstr& instr);

    void evalLiteral(double d);
    void evalSymbol(const SymbolNode& sym, const Env& current_env);
    void evalList(const std::vector<Expr>& list, const Env& env, const Address& addr);

    std::vector<Value> popArguments(int n);
    Value popValue();
    Address extendAddress(Address addr, const std::string& suffix);
    void applyFunction(const Value& func, const std::vector<Value>& args);
};