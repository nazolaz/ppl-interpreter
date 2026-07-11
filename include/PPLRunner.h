#ifndef PPLRUNNER_H
#define PPLRUNNER_H

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

class PPLRunner {
public:
    PPLRunner();
    int execute(int argc, char* argv[]);

private:
    std::string filename;
    std::string algorithm;
    int iterations;
    std::optional<uint32_t> seed;
    std::string outfile;
    bool dry_run;

    void print_usage(const std::string& program_name) const;
    bool parse_arguments(int argc, char* argv[]);
    void export_results(double mean, const std::vector<double>& results, const std::vector<double>& weights = {}) const;

    void run_lw();
    void run_smc();
    void run_mh();
};

#endif