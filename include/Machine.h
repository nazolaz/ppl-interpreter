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
    std::optional<Message> stepIfContinuation(IfkInstr& instr);
    std::optional<Message> stepSampleContinuation(SamplekInstr& instr);
    std::optional<Message> stepObserveContinuation(ObservekInstr& instr);
    
    void evalIfNode(const std::shared_ptr<IfNode>& if_node, const Env& env, const Address& addr);
    void evalLetNode(const std::shared_ptr<LetNode>& let_node, const Env& env, const Address& addr);
    void evalLiteral(double d);
    void evalSymbol(const SymbolNode& sym, const Env& current_env);
    void evalFnNode(const std::shared_ptr<FnNode>& fn_node, const Env& env);
    void evalList(const std::vector<Expr>& list, const Env& env, const Address& addr);

    std::vector<Value> popArguments(int n);
    Value popValue();
    Address extendAddress(Address addr, const std::string& suffix);
    void applyFunction(const Value& func, const std::vector<Value>& args, const Address& addr);
    void pushBody(const std::vector<Expr>& body, const Env& env, const Address& addr);
    void applyPrimitive(const Value& func, const std::vector<Value>& args);
    void applyClosure(const Value& func, const std::vector<Value>& args, const Address& addr);

};