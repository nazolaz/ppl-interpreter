#include "SequentialMonteCarlo.h"
#include "HOPPLParser.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <omp.h>

std::vector<double> SequentialMonteCarlo::run(const std::string& filename, int num_particles, uint32_t base_seed) {
    std::vector<Machine> particles;
    std::vector<AnyRNG> rngs;
    
    initialize_particles(filename, num_particles, base_seed, particles, rngs);

    while (true) {
        std::vector<Message> messages = advance_particles(particles, rngs);

        if (check_all_done(messages)) {
            return extract_results(messages);
        }

        if (!check_all_observe(messages)) {
            throw std::runtime_error("shared observe sequence needed.");
        }

        execute_resampling(particles, messages, rngs[0]);
    }
}

void SequentialMonteCarlo::initialize_particles(const std::string& filename, int num_particles, uint32_t base_seed, std::vector<Machine>& particles, std::vector<AnyRNG>& rngs) {
    HOPPLParser parser;
    Expr ast = parser.parse_file(filename);

    particles.resize(num_particles);
    rngs.reserve(num_particles);

    for (int i = 0; i < num_particles; ++i) {
        std::mt19937 base_engine(base_seed + i);
        rngs.emplace_back(base_engine);
        particles[i].load_ast(ast);
    }
}

std::vector<Message> SequentialMonteCarlo::advance_particles(std::vector<Machine>& particles, std::vector<AnyRNG>& rngs) {
    std::vector<Message> messages(particles.size());
    
    #pragma omp parallel for
    for (size_t i = 0; i < particles.size(); ++i) {
        messages[i] = advance(particles[i], rngs[i]);
    }
    
    return messages;
}

Message SequentialMonteCarlo::advance(Machine& m, AnyRNG& rng) {
    while (true) {
        Message msg = m.resume();
        if (std::holds_alternative<SampleMsg>(msg)) {
            auto sample_msg = std::get<SampleMsg>(msg);
            m.send(sample_msg.dist->sample(rng));
        } else {
            return msg;
        }
    }
}

bool SequentialMonteCarlo::check_all_done(const std::vector<Message>& messages) {
    for (const auto& msg : messages) {
        if (!std::holds_alternative<DoneMsg>(msg)) {
            return false;
        }
    }
    return true;
}

bool SequentialMonteCarlo::check_all_observe(const std::vector<Message>& messages) {
    for (const auto& msg : messages) {
        if (!std::holds_alternative<ObserveMsg>(msg)) {
            return false;
        }
    }
    return true;
}

std::vector<double> SequentialMonteCarlo::extract_results(const std::vector<Message>& messages) {
    std::vector<double> results;
    results.reserve(messages.size());
    
    for (const auto& msg : messages) {
        auto done_msg = std::get<DoneMsg>(msg);
        results.push_back(std::get<double>(done_msg.value));
    }
    
    return results;
}

void SequentialMonteCarlo::execute_resampling(std::vector<Machine>& particles, const std::vector<Message>& messages, AnyRNG& rng) {
    size_t num_particles = particles.size();
    std::vector<double> log_inc(num_particles);
    
    for (size_t i = 0; i < num_particles; ++i) {
        auto obs_msg = std::get<ObserveMsg>(messages[i]);
        double lp = obs_msg.dist->log_prob(obs_msg.value);
        
        particles[i].add_log_weight(lp);
        log_inc[i] = lp;
        particles[i].send(obs_msg.value);
    }

    std::vector<double> probabilities = softmax(log_inc);
    std::vector<size_t> ancestors = multinomial_resample(probabilities, num_particles, rng);

    std::vector<Machine> new_particles;
    new_particles.reserve(num_particles);
    
    for (size_t i = 0; i < num_particles; ++i) {
        new_particles.push_back(particles[ancestors[i]].fork());
    }
    
    particles = std::move(new_particles);
}

std::vector<double> SequentialMonteCarlo::softmax(const std::vector<double>& log_weights) {
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

std::vector<size_t> SequentialMonteCarlo::multinomial_resample(const std::vector<double>& probabilities, int N, AnyRNG& rng) {
    std::vector<size_t> indices(N);
    std::discrete_distribution<size_t> dist(probabilities.begin(), probabilities.end());
    
    for (int i = 0; i < N; ++i) {
        indices[i] = dist(rng);
    }
    
    return indices;
}