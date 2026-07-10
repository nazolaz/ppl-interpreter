#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include "Machine.h"
#include "AnyRNG.h"
#include "Expr.h"

struct AddressHash {
    std::size_t operator()(const Address& addr) const {
        std::size_t seed = addr.size();
        for (const auto& s : addr) {
            seed ^= std::hash<std::string>{}(s) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

struct MHState {
    double value;
    std::unordered_map<Address, double, AddressHash> X;
    std::unordered_map<Address, double, AddressHash> S;
    std::unordered_map<Address, double, AddressHash> O;
};

class MetropolisHastings {
public:
    std::vector<double> run(const std::string& filename, int steps, int warmup, std::optional<uint32_t> seed = std::nullopt);

private:
    Expr parse_program(const std::string& filename);
    MHState execute_trace(const Expr& ast, AnyRNG& rng, const std::optional<Address>& resample_addr, const std::unordered_map<Address, double, AddressHash>& cache);
    
    void handle_sample(Machine& m, const SampleMsg& msg, MHState& state, AnyRNG& rng, const std::optional<Address>& resample_addr, const std::unordered_map<Address, double, AddressHash>& cache);
    void handle_observe(Machine& m, const ObserveMsg& msg, MHState& state);
    void handle_done(const DoneMsg& msg, MHState& state);
    
    Address pick_resample_address(const std::unordered_map<Address, double, AddressHash>& trace, AnyRNG& rng);
    bool accept_proposal(double log_alpha, AnyRNG& rng);
    double compute_log_alpha(const MHState& curr, const MHState& prop, const Address& a0);
    
    std::unordered_set<Address, AddressHash> compute_fwd_set(const MHState& curr, const MHState& prop, const Address& a0);
    std::unordered_set<Address, AddressHash> compute_rev_set(const MHState& curr, const MHState& prop, const Address& a0);
    double compute_numerator(const MHState& prop, const std::unordered_set<Address, AddressHash>& fwd);
    double compute_denominator(const MHState& curr, const std::unordered_set<Address, AddressHash>& rev);
};