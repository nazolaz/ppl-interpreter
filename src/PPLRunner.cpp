#include "PPLRunner.h"
#include "LikelihoodWeighting.h"
#include "SequentialMonteCarlo.h"
#include "SSMetropolisHastings.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdexcept>

PPLRunner::PPLRunner() : iterations(10000), algorithm("lw"), outfile("") {}

void PPLRunner::print_usage(const std::string& program_name) const {
    std::cout << "Usage: " << program_name << " <filename.txt> [options]\n"
              << "Options:\n"
              << "  --algo <lw|smc|mh>   Inference algorithm (default: lw)\n"
              << "  --iter <number>      Number of particles/steps (default: 10000)\n"
              << "  --seed <number>      Deterministic seed (optional)\n"
              << "  --out <file.csv>     Export raw results to CSV (optional)\n";
}

bool PPLRunner::parse_arguments(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return false;
    }

    filename = argv[1];

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--algo" && i + 1 < argc) {
            algorithm = argv[++i];
        } else if (arg == "--iter" && i + 1 < argc) {
            iterations = std::stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            seed = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--out" && i + 1 < argc) {
            outfile = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            print_usage(argv[0]);
            return false;
        }
    }
    return true;
}

void PPLRunner::export_results(const std::vector<double>& results, const std::vector<double>& weights) const {
    if (outfile.empty()) return;

    std::ofstream file(outfile);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file " << outfile << " for writing.\n";
        return;
    }

    bool has_weights = !weights.empty() && weights.size() == results.size();
    file << "value" << (has_weights ? ",weight\n" : "\n");

    for (size_t i = 0; i < results.size(); ++i) {
        file << results[i];
        if (has_weights) file << "," << weights[i];
        file << "\n";
    }
    
    file.close();
    std::cout << "Results exported to " << outfile << "\n";
}

void PPLRunner::run_lw() {
    std::cout << "=> Starting Likelihood Weighting with " << iterations << " particles...\n";
    LikelihoodWeighting lw;
    auto [results, weights] = lw.run(filename, iterations, seed);
    
    double expected_value = 0.0;
    for (size_t i = 0; i < iterations; ++i) {
        expected_value += results[i] * weights[i];
    }
    std::cout << "Expected Value (Mean): " << expected_value << "\n";
    export_results(results, weights);
}

void PPLRunner::run_smc() {
    std::cout << "=> Starting SMC with " << iterations << " particles...\n";
    SequentialMonteCarlo smc;
    std::vector<double> results = smc.run(filename, iterations, seed);
    
    double expected_value = 0.0;
    for (double val : results) {
        expected_value += val;
    }
    expected_value /= iterations;
    std::cout << "Expected Value (Mean): " << expected_value << "\n";
    export_results(results);
}

void PPLRunner::run_mh() {
    std::cout << "=> Starting Metropolis-Hastings with " << iterations << " steps...\n";
    MetropolisHastings mh;
    int warmup = iterations / 10;
    std::vector<double> results = mh.run(filename, iterations, warmup, seed);
    
    double expected_value = 0.0;
    for (double val : results) {
        expected_value += val;
    }
    expected_value /= results.size();
    std::cout << "Expected Value (Mean post burn-in): " << expected_value << "\n";
    export_results(results);
}

int PPLRunner::execute(int argc, char* argv[]) {
    if (!parse_arguments(argc, argv)) return 1;
    try {
        if (algorithm == "lw") run_lw();
        else if (algorithm == "smc") run_smc();
        else if (algorithm == "mh") run_mh();
        else { std::cerr << "Unsupported algorithm: " << algorithm << "\n"; return 1; }
    } catch (const std::exception& e) {
        std::cerr << "Error during execution: " << e.what() << "\n";
        return 1;
    }
    return 0;
}