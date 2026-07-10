#include "SSMetropolisHastings.h"
#include "HOPPLParser.h"
#include <cmath>
#include <random>
#include <stdexcept>

std::vector<double> MetropolisHastings::run(const std::string& filename, int steps, int warmup, uint32_t seed) {
    Expr ast = parse_program(filename);
    
    std::mt19937 base_engine(seed);
    AnyRNG rng(base_engine);
    
    MHState current_state = execute_trace(ast, rng, std::nullopt, {});
    std::vector<double> chain;
    chain.reserve(steps);
    
    int total_iterations = steps + warmup;
    
    for (int i = 0; i < total_iterations; ++i) {
        Address a0 = pick_resample_address(current_state.X, rng);
        
        MHState proposed_state = execute_trace(ast, rng, a0, current_state.X);
        
        double log_alpha = compute_log_alpha(current_state, proposed_state, a0);
        
        if (accept_proposal(log_alpha, rng)) {
            current_state = std::move(proposed_state);
        }
        
        if (i >= warmup) {
            chain.push_back(current_state.value);
        }
    }
    
    return chain;
}

Expr MetropolisHastings::parse_program(const std::string& filename) {
    HOPPLParser parser;
    return parser.parse_file(filename);
}

MHState MetropolisHastings::execute_trace(const Expr& ast, AnyRNG& rng, const std::optional<Address>& resample_addr, const std::unordered_map<Address, double, AddressHash>& cache) {
    Machine m;
    m.load_ast(ast);
    MHState state;
    
    while (true) {
        Message msg = m.resume();
        
        if (std::holds_alternative<SampleMsg>(msg)) {
            handle_sample(m, std::get<SampleMsg>(msg), state, rng, resample_addr, cache);
        } else if (std::holds_alternative<ObserveMsg>(msg)) {
            handle_observe(m, std::get<ObserveMsg>(msg), state);
        } else if (std::holds_alternative<DoneMsg>(msg)) {
            handle_done(std::get<DoneMsg>(msg), state);
            return state;
        }
    }
}

void MetropolisHastings::handle_sample(Machine& m, const SampleMsg& msg, MHState& state, AnyRNG& rng, const std::optional<Address>& resample_addr, const std::unordered_map<Address, double, AddressHash>& cache) {
    double val;
    bool needs_resample = (resample_addr.has_value() && msg.addr == resample_addr.value());
    bool not_in_cache = (cache.find(msg.addr) == cache.end());
    
    if (needs_resample || not_in_cache) {
        val = msg.dist->sample(rng);
    } else {
        val = cache.at(msg.addr);
    }
    
    state.X[msg.addr] = val;
    state.S[msg.addr] = msg.dist->log_prob(val);
    m.send(val);
}

void MetropolisHastings::handle_observe(Machine& m, const ObserveMsg& msg, MHState& state) {
    state.O[msg.addr] = msg.dist->log_prob(msg.value);
    m.send(msg.value);
}

void MetropolisHastings::handle_done(const DoneMsg& msg, MHState& state) {
    state.value = std::get<double>(msg.value);
}

Address MetropolisHastings::pick_resample_address(const std::unordered_map<Address, double, AddressHash>& trace, AnyRNG& rng) {
    if (trace.empty()) {
        throw std::runtime_error("Trace is empty, cannot pick resample address.");
    }
    
    std::vector<Address> keys;
    keys.reserve(trace.size());
    for (const auto& pair : trace) {
        keys.push_back(pair.first);
    }
    
    std::uniform_int_distribution<size_t> dist(0, keys.size() - 1);
    return keys[dist(rng)];
}

bool MetropolisHastings::accept_proposal(double log_alpha, AnyRNG& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double u = dist(rng);
    return std::log(u) < log_alpha;
}

double MetropolisHastings::compute_log_alpha(const MHState& curr, const MHState& prop, const Address& a0) {
    std::unordered_set<Address, AddressHash> fwd = compute_fwd_set(curr, prop, a0);
    std::unordered_set<Address, AddressHash> rev = compute_rev_set(curr, prop, a0);
    
    double num = compute_numerator(prop, fwd);
    double den = compute_denominator(curr, rev);
    
    double trace_ratio = std::log(static_cast<double>(curr.X.size())) - std::log(static_cast<double>(prop.X.size()));
    
    return trace_ratio + (num - den);
}

std::unordered_set<Address, AddressHash> MetropolisHastings::compute_fwd_set(const MHState& curr, const MHState& prop, const Address& a0) {
    std::unordered_set<Address, AddressHash> fwd;
    fwd.insert(a0);
    
    for (const auto& pair : prop.X) {
        if (curr.X.find(pair.first) == curr.X.end()) {
            fwd.insert(pair.first);
        }
    }
    
    return fwd;
}

std::unordered_set<Address, AddressHash> MetropolisHastings::compute_rev_set(const MHState& curr, const MHState& prop, const Address& a0) {
    std::unordered_set<Address, AddressHash> rev;
    rev.insert(a0);
    
    for (const auto& pair : curr.X) {
        if (prop.X.find(pair.first) == prop.X.end()) {
            rev.insert(pair.first);
        }
    }
    
    return rev;
}

double MetropolisHastings::compute_numerator(const MHState& prop, const std::unordered_set<Address, AddressHash>& fwd) {
    double num = 0.0;
    
    for (const auto& pair : prop.S) {
        if (fwd.find(pair.first) == fwd.end()) {
            num += pair.second;
        }
    }
    
    for (const auto& pair : prop.O) {
        num += pair.second;
    }
    
    return num;
}

double MetropolisHastings::compute_denominator(const MHState& curr, const std::unordered_set<Address, AddressHash>& rev) {
    double den = 0.0;
    
    for (const auto& pair : curr.S) {
        if (rev.find(pair.first) == rev.end()) {
            den += pair.second;
        }
    }
    
    for (const auto& pair : curr.O) {
        den += pair.second;
    }
    
    return den;
}