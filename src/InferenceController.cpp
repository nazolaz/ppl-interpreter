#include "../include/InferenceController.h"

InferenceController::InferenceController(AnyRNG generator) : rng(std::move(generator)) {}

void InferenceController::run(const std::string& filename, int iterations) {
    HOPPLParser parser;
    Expr ast = parser.parse_file(filename);
    this->run_ast(ast, iterations);
}

LikelihoodWeightingController::LikelihoodWeightingController(AnyRNG generator) 
    : InferenceController(std::move(generator)) {}

void LikelihoodWeightingController::run_ast(const Expr& ast, int iterations) {
}