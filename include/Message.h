#pragma once

#include <variant>
#include <memory>
#include "Machine.h" 

struct SampleMsg {
    Address addr;
    std::shared_ptr<Distribution> dist;
};

struct ObserveMsg {
    Address addr;
    std::shared_ptr<Distribution> dist;
    double value;
};

struct DoneMsg {
    Value value;
};