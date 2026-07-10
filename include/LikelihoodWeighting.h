#pragma once
#include <string>
#include <vector>
#include <utility>
#include "Value.h"
#include "Machine.h"
#include "AnyRNG.h"

class LikelihoodWeighting {
private:
    std::pair<Value, double> run_particle(Machine& m, AnyRNG& rng);
    std::vector<double> softmax(const std::vector<double>& log_weights);

public:
    LikelihoodWeighting() = default;
    
    void run(const std::string& filename, int iterations, uint32_t seed = std::random_device{}());
};