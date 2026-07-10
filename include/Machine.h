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
    std::optional<Message> stepLetContinuation(LetkInstr& instr);

    void evalLetNode(const std::shared_ptr<LetNode>& let_node, const Env& env, const Address& addr);
    void evalLiteral(double d);
    void evalSymbol(const SymbolNode& sym, const Env& current_env);
    void evalList(const std::vector<Expr>& list, const Env& env, const Address& addr);

    std::vector<Value> popArguments(int n);
    Value popValue();
    Address extendAddress(Address addr, const std::string& suffix);
    void applyFunction(const Value& func, const std::vector<Value>& args);
    void pushBody(const std::vector<Expr>& body, const Env& env, const Address& addr);
    bool hasBinding(const std::vector<Expr>& binds, int pair_index);
    std::string getBindingName(const std::vector<Expr>& binds, int pair_index);
    Expr getBindingValue(const std::vector<Expr>& binds, int pair_index);
    bool isPrimitive(const Value& func);
    std::string getPrimitiveName(const Value& func);
};