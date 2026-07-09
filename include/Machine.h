#pragma once
#include <vector>
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
};