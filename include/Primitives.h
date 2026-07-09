#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include "Value.h"

using PrimitiveFunc = std::function<Value(const std::vector<Value>&)>;

extern std::unordered_map<std::string, PrimitiveFunc> PRIMITIVES;

bool is_primitive(const std::string& name);