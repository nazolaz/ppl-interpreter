#pragma once

#include <vector>
#include <string>
#include <memory>
#include <variant>
#include "Expr.h"
#include "Value.h"

struct EvInstr { Expr e; Env env; Address addr; };
struct IfkInstr { std::shared_ptr<Expr> then_branch; std::shared_ptr<Expr> else_branch; Env env; Address addr; };
struct LetkInstr { std::vector<Expr> binds; int i; std::vector<Expr> body; Env env; Address addr; };
struct CallkInstr { int n_args; Address addr; };
struct SamplekInstr { Address addr; };
struct ObservekInstr { Address addr; };
struct DiscardInstr {};

using Instruction = std::variant<
    EvInstr, IfkInstr, LetkInstr, CallkInstr, 
    SamplekInstr, ObservekInstr, DiscardInstr
>;