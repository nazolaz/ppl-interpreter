#pragma once
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include "Value.h"
#include "Distribution.h"

struct SampleMsg {
    std::vector<std::string> addr;
    std::shared_ptr<Distribution> dist;
};

struct ObserveMsg {
    std::vector<std::string> addr;
    std::shared_ptr<Distribution> dist;
    double value;
};

struct DoneMsg {
    Value value;
};

using Message = std::variant<SampleMsg, ObserveMsg, DoneMsg>;