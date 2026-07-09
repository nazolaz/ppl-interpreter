#pragma once
#include <variant>
#include <memory>
#include "Value.h"

struct Machine;
struct Address; 
using Message = std::variant<SampleMsg, ObserveMsg, DoneMsg>;

struct SampleMsg {
    std::vector<std::string> addr;
    std::shared_ptr<Distribution> dist;
    Machine* m;
};

struct ObserveMsg {
    std::vector<std::string> addr;
    std::shared_ptr<Distribution> dist;
    double value;
    Machine* m;
};

struct DoneMsg {
    Value value;
    Machine* m;
};

