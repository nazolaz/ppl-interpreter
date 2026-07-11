#pragma once

#include <string>
#include <optional>
#include <vector>
#include <cstdint>

class PPLRunner {
private:
    std::string filename;
    std::string algorithm;
    int iterations;
    std::optional<uint32_t> seed;
    std::string outfile; 

    void print_usage(const std::string& program_name) const;
    bool parse_arguments(int argc, char* argv[]);
    void export_results(const std::vector<double>& results, const std::vector<double>& weights = {}) const;
    
    void run_lw();
    void run_smc();
    void run_mh();

public:
    PPLRunner();
    int execute(int argc, char* argv[]);
};