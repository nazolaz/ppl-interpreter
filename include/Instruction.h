#pragma once

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include "Expr.h"
#include "Value.h"

struct EvInstr { Expr e; Env env; Address addr; };
struct IfkInstr { std::shared_ptr<Expr> then_branch; std::shared_ptr<Expr> else_branch; Env env; Address addr; };
struct CallkInstr { int n_args; Address addr; };
struct SamplekInstr { Address addr; };
struct ObservekInstr { Address addr; };
struct DiscardInstr {};
struct LetkInstr { 
    std::vector<Expr> binds; 
    int i; 
    std::vector<Expr> body; 
    Env env; 
    Address addr; 

    bool hasBinding(int pair_index) const { return (2 * pair_index) < binds.size(); }
    std::string getBindingName(int pair_index) const { return std::get<SymbolNode>(binds[2 * pair_index].value).name; }
    Expr getBindingValue(int pair_index) const { return binds[(2 * pair_index) + 1]; }
};

using Instruction = std::variant<
    EvInstr, IfkInstr, LetkInstr, CallkInstr, 
    SamplekInstr, ObservekInstr, DiscardInstr
>;