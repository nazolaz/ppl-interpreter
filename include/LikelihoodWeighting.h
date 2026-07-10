#pragma once

#include <string>
#include <vector>
#include <utility>
#include "Machine.h"
#include "AnyRNG.h"
#include "Expr.h"
#include "Value.h"

class LikelihoodWeighting {
public:
    void run(const std::string& filename, int iterations, uint32_t seed);

private:
    Expr parse_program(const std::string& filename);
    void execute_parallel_particles(const Expr& ast, int iterations, uint32_t seed, std::vector<Value>& results, std::vector<double>& log_weights);
    std::pair<Value, double> run_particle(Machine& m, AnyRNG& rng);
    std::vector<double> softmax(const std::vector<double>& log_weights);
};