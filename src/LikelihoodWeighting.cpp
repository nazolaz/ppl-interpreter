#include "LikelihoodWeighting.h"
#include "HOPPLParser.h" 
#include <cmath>
#include <algorithm>
#include <random>
#include <omp.h>

std::pair<std::vector<double>, std::vector<double>> LikelihoodWeighting::run(const std::string& filename, int iterations, std::optional<uint32_t> seed) {
    Expr ast = parse_program(filename);

    std::vector<Value> results(iterations);
    std::vector<double> log_weights(iterations);

    execute_parallel_particles(ast, iterations, seed, results, log_weights);

    std::vector<double> normalized_weights = softmax(log_weights);
    std::vector<double> numeric_results(iterations);

    for (int i = 0; i < iterations; ++i) {
        numeric_results[i] = std::get<double>(results[i]);
    }

    return {numeric_results, normalized_weights};
}

Expr LikelihoodWeighting::parse_program(const std::string& filename) {
    HOPPLParser parser;
    return parser.parse_file(filename);
}

void LikelihoodWeighting::execute_parallel_particles(const Expr& ast, int iterations, std::optional<uint32_t> seed, std::vector<Value>& results, std::vector<double>& log_weights) {
    #pragma omp parallel for
    for (int i = 0; i < iterations; ++i) {
        Machine m;
        m.load_ast(ast); 
        
        std::pair<Value, double> particle_result;

        if (seed.has_value()) {
            std::mt19937 base_rng(seed.value() + i);
            AnyRNG rng(base_rng);
            particle_result = run_particle(m, rng);
        } else {
            AnyRNG rng;
            particle_result = run_particle(m, rng);
        }
        
        results[i] = particle_result.first;
        log_weights[i] = particle_result.second;
    }
}

std::pair<Value, double> LikelihoodWeighting::run_particle(Machine& m, AnyRNG& rng) {
    while (true) {
        Message msg = m.resume();

        if (std::holds_alternative<DoneMsg>(msg)) {
            auto done = std::get<DoneMsg>(msg);
            return {done.value, m.get_log_weight()}; 
        } 
        else if (std::holds_alternative<SampleMsg>(msg)) {
            auto sample_msg = std::get<SampleMsg>(msg);
            double val = sample_msg.dist->sample(rng);
            
            m.send(val); 
        } 
        else if (std::holds_alternative<ObserveMsg>(msg)) {
            auto obs_msg = std::get<ObserveMsg>(msg);
            
            m.add_log_weight(obs_msg.dist->log_prob(obs_msg.value)); 
            m.send(obs_msg.value); 
        }
    }
}

std::vector<double> LikelihoodWeighting::softmax(const std::vector<double>& log_weights) {
    if (log_weights.empty()) return {};
    
    double max_log_w = *std::max_element(log_weights.begin(), log_weights.end());
    
    std::vector<double> probs(log_weights.size());
    double sum = 0.0;
    
    for (size_t i = 0; i < log_weights.size(); ++i) {
        probs[i] = std::exp(log_weights[i] - max_log_w);
        sum += probs[i];
    }
    
    for (size_t i = 0; i < probs.size(); ++i) {
        probs[i] /= sum;
    }
    
    return probs;
}