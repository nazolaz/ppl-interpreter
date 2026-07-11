#include "PPLRunner.h"
#include "LikelihoodWeighting.h"
#include "SequentialMonteCarlo.h"
#include "SSMetropolisHastings.h"
#include "HOPPLParser.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <vector>
#include <string>

PPLRunner::PPLRunner() : iterations(10000), algorithm("lw"), outfile(""), dry_run(false) {}

void PPLRunner::print_usage(const std::string& program_name) const {
    std::cout << "Usage: " << program_name << " <filename.txt> [options]\n"
              << "Options:\n"
              << "  --algo <lw|smc|mh>   Inference algorithm (default: lw)\n"
              << "  --iter <number>      Number of particles/steps (default: 10000)\n"
              << "  --seed <number>      Deterministic seed (optional)\n"
              << "  --out <file.csv>     Export raw results to CSV (optional)\n"
              << "  --dry-run            Parse and validate syntax without running inference\n";
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
        } else if (arg == "--dry-run") {
            dry_run = true;
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            print_usage(argv[0]);
            return false;
        }
    }
    return true;
}

void PPLRunner::export_results(double mean, const std::vector<double>& results, const std::vector<double>& weights) const {
    if (outfile.empty()) return;

    std::filesystem::path out_path(outfile);
    
    if (!out_path.has_parent_path()) {
        std::filesystem::create_directory("outputs");
        out_path = std::filesystem::path("outputs") / out_path;
    } else {
        std::filesystem::create_directories(out_path.parent_path());
    }

    std::ofstream f(out_path);
    
    f << "# Mean: " << mean << "\n";
    
    f << "step,algorithm,seed,value";
    if (!weights.empty()) {
        f << ",weight";
    }
    f << "\n";

    std::string seed_str = seed.has_value() ? std::to_string(seed.value()) : "random";

    for (size_t i = 0; i < results.size(); ++i) {
        f << i << "," << algorithm << "," << seed_str << "," << results[i];
        if (!weights.empty()) {
            f << "," << weights[i];
        }
        f << "\n";
    }
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
    export_results(expected_value, results, weights);
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
    export_results(expected_value, results);
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
    export_results(expected_value, results);
}

int PPLRunner::execute(int argc, char* argv[]) {
    if (!parse_arguments(argc, argv)) return 1;
    try {
        if (dry_run) {
            std::cout << "=> Performing dry run for syntax validation...\n";
            HOPPLParser::parse_file(filename);
            std::cout << "=> Dry run successful. Syntax is valid.\n";
            return 0;
        }
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