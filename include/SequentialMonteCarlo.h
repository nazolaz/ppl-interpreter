#pragma once

#include <string>
#include <vector>
#include "Machine.h"
#include "AnyRNG.h"
#include "Message.h"

class SequentialMonteCarlo {
public:
    std::vector<double> run(const std::string& filename, int num_particles, uint32_t base_seed);

private:
    void initialize_particles(const std::string& filename, int num_particles, uint32_t base_seed, std::vector<Machine>& particles, std::vector<AnyRNG>& rngs);
    std::vector<Message> advance_particles(std::vector<Machine>& particles, std::vector<AnyRNG>& rngs);
    Message advance(Machine& m, AnyRNG& rng);
    bool check_all_done(const std::vector<Message>& messages);
    bool check_all_observe(const std::vector<Message>& messages);
    std::vector<double> extract_results(const std::vector<Message>& messages);
    void execute_resampling(std::vector<Machine>& particles, const std::vector<Message>& messages, AnyRNG& rng);
    std::vector<double> softmax(const std::vector<double>& log_weights);
    std::vector<size_t> multinomial_resample(const std::vector<double>& probabilities, int N, AnyRNG& rng);
};